/* Copyright (C) 2004-2015 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include "mbsim/gear.h"
#include "mbsim/fixed_relative_frame.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/rigid_body.h"
#include "mbsim/constraint.h"
#include <openmbvcppinterface/group.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;
using namespace boost;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Gear, MBSIM%"Gear")

  Gear::Gear(const string &name) : RigidBodyLink(name), func(0) {
    connect(NULL, -1);
  }

  void Gear::addTransmission(const Transmission &transmission) { 
    connect(transmission.body, transmission.ratio);
  }

  void Gear::updateGeneralizedSetValuedForces(double t) {
    laMV = la;
    updlaMV = false;
  }

  void Gear::updateGeneralizedSingleValuedForces(double t) {
    laSV(0) = -(*func)(getGeneralizedRelativePosition(t)(0),getGeneralizedRelativeVelocity(t)(0));
    updlaSV = false;
  }

 void Gear::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if (saved_DependentBody!="")
        setDependentBody(getByPath<RigidBody>(saved_DependentBody));
      if (saved_IndependentBody.size()>0) {
        for (unsigned int i=0; i<saved_IndependentBody.size(); i++)
          body.push_back(getByPath<RigidBody>(saved_IndependentBody[i]));
      }
      RigidBodyLink::init(stage);
    }
    else if(stage==resize) {
      RigidBodyLink::init(stage);
      rrel.resize(1);
      vrel.resize(1);
      if(isSetValued()) {
        g.resize(1);
        gd.resize(1);
        la.resize(1);
        laMV.resize(1);
      }
      else
        laSV.resize(1);
    }
    else {
      RigidBodyLink::init(stage);
    }
    if(func) func->init(stage);
  }

  void Gear::initializeUsingXML(DOMElement* element) {
    RigidBodyLink::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"generalizedForceFunction");
    if(e) {
      Function<double(double,double)> *f=ObjectFactory::createAndInit<Function<double(double,double)> >(e->getFirstElementChild());
      setGeneralizedForceFunction(f);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBody");
    saved_DependentBody=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIM%"transmissions");
    DOMElement *ee=e->getFirstElementChild();
    while(ee && E(ee)->getTagName()==MBSIM%"Transmission") {
      saved_IndependentBody.push_back(E(E(ee)->getFirstElementChildNamed(MBSIM%"rigidBody"))->getAttribute("ref"));
      ratio.push_back(getDouble(E(ee)->getFirstElementChildNamed(MBSIM%"ratio")));
      ee=ee->getNextElementSibling();
    }
  }

}


