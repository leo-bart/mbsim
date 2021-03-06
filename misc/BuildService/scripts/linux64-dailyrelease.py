#!/usr/bin/python

from __future__ import print_function # to enable the print function for backward compatiblity with python2
import os
import subprocess

SRCDIR="/home/mbsim/linux64-dailyrelease"
OUTDIR="/var/www/html/mbsim/linux64-dailyrelease"
URL="https://www.mbsim-env.de/mbsim/linux64-dailyrelease"
SCRIPTDIR=os.path.dirname(os.path.realpath(__file__))


os.environ["PKG_CONFIG_PATH"]=SRCDIR+"/local/lib/pkgconfig:/home/mbsim/3rdparty/casadi3py-local-linux64/lib/pkgconfig:"+ \
  "/home/mbsim/3rdparty/coin-local-linux64/lib/pkgconfig"
os.environ["LD_LIBRARY_PATH"]="/home/mbsim/3rdparty/casadi3py-local-linux64/lib"
os.environ["CXXFLAGS"]="-g -O2"
os.environ["CFLAGS"]="-g -O2"
os.environ["FFLAGS"]="-g -O2"
os.environ['MBSIM_SWIG']='1'

ret=subprocess.call([SCRIPTDIR+"/build.py", "--buildSystemRun", "--enableDistribution", "--rotate", "30", "-j", "2", "--sourceDir", SRCDIR, "--webapp",
  "--prefix", SRCDIR+"/local", "--reportOutDir", OUTDIR+"/report", "--url", URL+"/report", "--buildType", "linux64-dailyrelease",
  "--enableCleanPrefix", "--passToConfigure", "--with-lowram", "--enable-python", "--enable-shared", "--disable-static", "--with-qwt-inc-prefix=/usr/include/qwt",
  "--with-swigpath=/home/mbsim/3rdparty/swig-local-linux64/bin", "--with-qmake=qmake-qt4",
  "--passToRunexamples", "--disableCompare", "--disableValidate", "--filter", "'basic' in labels"])
if ret!=0 and ret!=255:
  print("linux64-dailyrelease failed.")
