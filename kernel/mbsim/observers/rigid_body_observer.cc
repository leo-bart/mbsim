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
#include "mbsim/observers/rigid_body_observer.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/links/joint.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/utils/eps.h"
#include <openmbvcppinterface/frame.h>
#include <openmbvcppinterface/group.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, RigidBodyObserver)

  RigidBodyObserver::RigidBodyObserver(const std::string &name) : Observer(name), body(NULL) {
  }

  void RigidBodyObserver::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_body!="")
        setRigidBody(getByPath<RigidBody>(saved_body));
      Observer::init(stage);
    }
    else if(stage==plotting) {
      updatePlotFeatures();

      Observer::init(stage);
      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(openMBV)==enabled) {
          if(FWeight) {
            FWeight->setName("Weight");
            getOpenMBVGrp()->addObject(FWeight);
          }
          if(FArrow) {
            FArrow->setName("JointForce");
            getOpenMBVGrp()->addObject(FArrow);
          }
          if(MArrow) {
            MArrow->setName("JointMoment");
            getOpenMBVGrp()->addObject(MArrow);
          }
          if(openMBVAxisOfRotation) {
            openMBVAxisOfRotation->setName("AxisOfRotation");
            getOpenMBVGrp()->addObject(openMBVAxisOfRotation);
          }
        }
      }
    }
    else
      Observer::init(stage);
  }

  void RigidBodyObserver::plot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(openMBV)==enabled) {
        if(FWeight) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 WrOS=body->getFrameC()->evalPosition();
          Vec3 WG = body->getMass()*MBSimEnvironment::getInstance()->getAccelerationOfGravity();
          data.push_back(WrOS(0));
          data.push_back(WrOS(1));
          data.push_back(WrOS(2));
          data.push_back(WG(0));
          data.push_back(WG(1));
          data.push_back(WG(2));
          data.push_back(1.0);
          FWeight->append(data);
        }
        if(FArrow) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 toPoint=body->getJoint()->getFrame(1)->evalPosition();
          data.push_back(toPoint(0));
          data.push_back(toPoint(1));
          data.push_back(toPoint(2));
          Vec3 WF = body->getJoint()->evalForce();
          data.push_back(WF(0));
          data.push_back(WF(1));
          data.push_back(WF(2));
          data.push_back(nrm2(WF));
          FArrow->append(data);
        }
        if(MArrow) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 toPoint=body->getJoint()->getFrame(1)->evalPosition();
          data.push_back(toPoint(0));
          data.push_back(toPoint(1));
          data.push_back(toPoint(2));
          Vec3 WM = body->getJoint()->evalMoment();
          data.push_back(WM(0));
          data.push_back(WM(1));
          data.push_back(WM(2));
          data.push_back(nrm2(WM));
          MArrow->append(data);
        }
        if(openMBVAxisOfRotation) {
          Vec3 r = body->getFrameC()->evalPosition();
          Vec3 om = body->getFrame("C")->evalAngularVelocity();
          Vec3 a;
          double nrmom = nrm2(om);
          if(nrmom > epsroot())
            a = om/nrmom;
          vector<double> data;
          data.push_back(getTime());
          data.push_back(r(0));
          data.push_back(r(1));
          data.push_back(r(2));
          data.push_back(a(0));
          data.push_back(a(1));
          data.push_back(a(2));
          data.push_back(0.5);
          openMBVAxisOfRotation->append(data);
        }
      }
      Observer::plot();
    }
  }
  
  void RigidBodyObserver::initializeUsingXML(DOMElement *element) {
    Observer::initializeUsingXML(element);

    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"rigidBody");
    if(e) saved_body=E(e)->getAttribute("ref");

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVWeight");
    if(e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FWeight=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointForce");
    if(e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointMoment");
    if(e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
  }

}
