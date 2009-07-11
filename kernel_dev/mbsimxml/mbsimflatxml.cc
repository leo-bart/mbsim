#include "config.h"
#include <iostream>
#include "mbsimtinyxml/tinyxml-src/tinyxml.h"
#include "mbsimtinyxml/tinyxml-src/tinynamespace.h"
#include "mbsim/objectfactory.h"
#include "mbsim/dynamic_system_solver.h"
#include <mbsim/integrators/integrator.h>

using namespace std;
using namespace MBSim;

int main(int argc, char *argv[]) {
  // help
  if(argc!=3) {
    cout<<"Usage: mbximxml <mbsimfile> <mbsimintegratorfile>"<<endl;
    cout<<endl;
    cout<<"Copyright (C) 2004-2009 MBSim Development Team"<<endl;
    cout<<"This is free software; see the source for copying conditions. There is NO"<<endl;
    cout<<"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."<<endl;
    cout<<endl;
    cout<<"Licensed under the GNU Lesser General Public License (LGPL)"<<endl;
    return 0;
  }



  // load MBSim XML document
  TiXmlDocument *doc=new TiXmlDocument;
  if(doc->LoadFile(argv[1])==false) {
    cerr<<"ERROR! Unable to load file: "<<argv[1]<<endl;
    return 1;
  }
  TiXmlElement *e=doc->FirstChildElement();
  map<string,string> dummy;
  incorporateNamespace(e, dummy);

  // create object for root element and check correct type
  DynamicSystemSolver *dss=dynamic_cast<DynamicSystemSolver*>(ObjectFactory::createGroup(e));
  if(dss==0) {
    cerr<<"ERROR! The root element of the MBSim main file must be of type 'DynamicSystemSolver'"<<endl;
    return 1;
  }
  dss->initializeUsingXML(e);
  delete doc;
  dss->init();



  // load MBSimIntegrator XML document
  doc=new TiXmlDocument;
  if(doc->LoadFile(argv[2])==false) {
    cerr<<"ERROR! Unable to load file: "<<argv[2]<<endl;
    return 1;
  }
  e=doc->FirstChildElement();
  incorporateNamespace(e, dummy);

  // create integrator
  Integrator *integrator=ObjectFactory::createIntegrator(e);
  if(integrator==0) {
    cerr<<"ERROR! Cannot create the integrator object!"<<endl;
    return 1;
  }
  integrator->initializeUsingXML(e);
  delete doc;
  integrator->integrate(*dss);
  delete integrator;

  return 0;
}
