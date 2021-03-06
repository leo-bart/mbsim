#include "config.h"
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <string.h>
#include <fstream>
#include <mbxmlutilshelper/getinstallpath.h>
#include <mbxmlutilshelper/last_write_time.h>
#include <mbsim/element.h>
#include <mbsim/integrators/integrator.h>
#include <stdio.h>
#if !defined _WIN32
#  include <spawn.h>
#  include <sys/types.h>
#  include <sys/wait.h>
#else
#  include <process.h>
#  include <windows.h>
#endif
#include <mbxmlutilshelper/dom.h>
#include <xercesc/dom/DOMDocument.hpp>
#include "mbsimxml.h"

using namespace std;
using namespace MBXMLUtils;
namespace bfs=boost::filesystem;

int runProgram(const vector<string> &arg) {
  // convert arg to c style
  char **argv=new char*[arg.size()+1];
  for(size_t i=0; i<arg.size(); i++)
    argv[i]=const_cast<char*>(arg[i].c_str());
  argv[arg.size()]=NULL;

#if !defined _WIN32
  pid_t child;
  int spawnRet;
  // Use posix_spawn from a older glibc implementation to allow binary distributions to run on older systems
  #if defined(__GNUC__) // GNU compiler
    #if !defined(__LP64__) // 32 bit
      __asm__(".symver posix_spawn, posix_spawn@GLIBC_2.2");
    #else // 64 bit
      __asm__(".symver posix_spawn, posix_spawn@GLIBC_2.2.5");
    #endif
  #endif
  spawnRet=posix_spawn(&child, argv[0], NULL, NULL, argv, environ);
  delete[]argv;
  if(spawnRet!=0)
    throw runtime_error("Unable to spawn process.");
  int status;
  waitpid(child, &status, 0);
  if(WIFEXITED(status))
    return WEXITSTATUS(status);
  else
    throw runtime_error("Spawn process terminated abnormally.");
#else
  int ret;
  ret=_spawnv(_P_WAIT, argv[0], argv);
  delete[]argv;
  if(ret==-1)
    throw runtime_error("Unable to spawn process.");
  return ret;
#endif
}

// return true if filanamea is newer then filenameb (modification time) (OS-independent)
bool newer(const string &filenamea, const string &filenameb) {
  if(!bfs::exists(filenamea.c_str()) || !bfs::exists(filenameb.c_str()))
    return false;

  return boost::myfilesystem::last_write_time(filenamea.c_str())>
         boost::myfilesystem::last_write_time(filenameb.c_str());
}


// return filename without path but with extension (OS-independent)
string basename(const string &filename) {
  bfs::path p(filename.c_str());
  return p.filename().generic_string();
}

// create filename if it does not exist or touch it if if exists (OS-independent)
void createOrTouch(const string &filename) {
  ofstream f(filename.c_str()); // Note: ofstream use precise file timestamp
}

int main(int argc, char *argv[]) {
  try {

    // convert args to c++
    list<string> arg;
    list<string>::iterator i, i2;
    for(int i=1; i<argc; i++)
      arg.push_back(argv[i]);

    bool ONLYLISTSCHEMAS=std::find(arg.begin(), arg.end(), "--onlyListSchemas")!=arg.end();
  
    // help
    if(arg.size()<1 ||
       std::find(arg.begin(), arg.end(), "-h")!=arg.end() ||
       std::find(arg.begin(), arg.end(), "--help")!=arg.end() ||
       std::find(arg.begin(), arg.end(), "-?")!=arg.end()) {
      cout<<"Usage: mbsimxml [--onlypreprocess|--donotintegrate|--stopafterfirststep|"<<endl
          <<"                 --autoreload|--onlyListSchemas]"<<endl
          <<"                [--modulesPath <dir> [--modulePath <dir> ...]]"<<endl
          <<"                <mbsimprjfile>"<<endl
          <<""<<endl
          <<"Copyright (C) 2004-2009 MBSim Development Team"<<endl
          <<"This is free software; see the source for copying conditions. There is NO"<<endl
          <<"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl
          <<""<<endl
          <<"Licensed under the GNU Lesser General Public License (LGPL)"<<endl
          <<""<<endl
          <<"--onlypreprocess     Stop after the preprocessing stage"<<endl
          <<"--donotintegrate     Stop after the initialization stage, do not integrate"<<endl
          <<"--stopafterfirststep Stop after outputting the first step (usually at t=0)"<<endl
          <<"                     This generates a HDF5 output file with only one time serie"<<endl
          <<"--autoreload         Same as --stopafterfirststep but rerun mbsimxml each time"<<endl
          <<"                     a input file is newer than the output file"<<endl
          <<"--onlyListSchemas    List all XML schema files including MBSim modules"<<endl
          <<"--modulePath <dir>   Add <dir> to MBSim module serach path. The central MBSim installation"<<endl
          <<"                     module dir and the current dir is always included."<<endl
          <<"<mbsimprjfile>       Use <mbsimprjfile> as mbsim xml project file"<<endl;
      return 0;
    }
  
    // get path of this executable
    string EXEEXT;
#ifdef _WIN32 // Windows
    EXEEXT=".exe";
#else // Linux
    EXEEXT="";
#endif
  
    bfs::path MBXMLUTILSBIN=getInstallPath()/"bin";
    bfs::path MBXMLUTILSSCHEMA=getInstallPath()/"share"/"mbxmlutils"/"schema";
  
    // parse parameters

    // generate mbsimxml.xsd
    set<bfs::path> searchDirs;
    while((i=find(arg.begin(), arg.end(), "--modulePath"))!=arg.end()) {
      i2=i; i2++;
      searchDirs.insert(*i2);
      arg.erase(i);
      arg.erase(i2);
    }
    set<bfs::path> schemas=MBSim::getMBSimXMLSchemas(searchDirs);
    if(ONLYLISTSCHEMAS) {
      for(auto &schema: schemas)
        cout<<schema.string()<<endl;
      return 0;
    }
  
    bool ONLYPP=false;
    if((i=std::find(arg.begin(), arg.end(), "--onlypreprocess"))!=arg.end()) {
      ONLYPP=true;
      arg.erase(i);
    }
  
    string NOINT;
    if((i=std::find(arg.begin(), arg.end(), "--donotintegrate"))!=arg.end()) {
      NOINT="--donotintegrate";
      arg.erase(i);
    }
  
    string ONLY1OUT;
    if((i=std::find(arg.begin(), arg.end(), "--stopafterfirststep"))!=arg.end()) {
      ONLY1OUT="--stopafterfirststep";
      arg.erase(i);
    }
  
    int AUTORELOADTIME=0;
    if((i=std::find(arg.begin(), arg.end(), "--autoreload"))!=arg.end()) {
      i2=i; i2++;
      char *error;
      ONLY1OUT="--stopafterfirststep";
      AUTORELOADTIME=strtol(i2->c_str(), &error, 10);
      // AUTORELOAD is set delayed since we currently do not know the MBSim file name (see later)
      if(AUTORELOADTIME<0) AUTORELOADTIME=250;
      arg.erase(i);
      if(error && strlen(error)==0)
        arg.erase(i2);
      else
        AUTORELOADTIME=250;
    }
  
    string MBSIMPRJ=*arg.begin();
    arg.erase(arg.begin());
    string PPMBSIMPRJ=".pp."+basename(MBSIMPRJ);
    string DEPMBSIMPRJ=".dep."+basename(MBSIMPRJ);
    string ERRFILE=".err."+basename(MBSIMPRJ);
  
    vector<string> AUTORELOAD;
    if(AUTORELOADTIME>0) { // AUTORELOAD is now set (see above)
      AUTORELOAD.push_back("--dependencies");
      AUTORELOAD.push_back(DEPMBSIMPRJ);
    }
  
    // execute
    int ret; // comamnd return value
    bool runAgain=true; // always run the first time
    while(runAgain) {
      unlink(PPMBSIMPRJ.c_str());
      unlink(ERRFILE.c_str());
  
      // run preprocessor
      // validate the project file with mbsimxml.xsd
      vector<string> command;
      command.push_back((MBXMLUTILSBIN/(string("mbxmlutilspp")+EXEEXT)).string());
      command.insert(command.end(), AUTORELOAD.begin(), AUTORELOAD.end());
      for(auto &schema: schemas)
        command.push_back(schema.string());
      command.push_back(MBSIMPRJ);
      ret=runProgram(command);
  
      if(!ONLYPP && ret==0) {
        vector<string> command;
        command.push_back((MBXMLUTILSBIN/(string("mbsimflatxml")+EXEEXT)).string());
        if(NOINT!="") command.push_back(NOINT);
        if(ONLY1OUT!="") command.push_back(ONLY1OUT);
        command.push_back(PPMBSIMPRJ);
        ret=runProgram(command);
      }
  
      if(ret!=0) createOrTouch(ERRFILE);
  
      runAgain=false; // only run ones except --autoreload is given
      if(AUTORELOADTIME>0) {
        // get dependent files
        vector<string> depfiles;
        ifstream deps(DEPMBSIMPRJ.c_str());
        while(true) {
          string depfile;
          getline(deps, depfile);
          if(deps.fail()) break;
          depfiles.push_back(depfile);
        }
        deps.close();
        // check for dependent files being newer then the output file
        while(!runAgain) {
#ifndef _WIN32
          usleep(AUTORELOADTIME*1000);
#else
          Sleep(AUTORELOADTIME);
#endif
          for(size_t i=0; i<depfiles.size(); i++) {
            if(newer(depfiles[i], PPMBSIMPRJ) || newer(depfiles[i], ERRFILE)) {
              runAgain=true;
              break;
            }
          }
        }
      }
    }
  
    return ret;
  }
  catch(const exception &e) {
    cerr<<"Exception:"<<endl
        <<e.what()<<endl;
    return 1;
  }
  catch(...) {
    cerr<<"Unknown exception"<<endl;
    return 1;
  }
  return 0;
}
