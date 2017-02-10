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
#include "mbsim/links/rigid_body_link.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/utils/utils.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  RigidBodyLink::RigidBodyLink(const string &name) : MechanicalLink(name), updPos(true), updVel(true), updFD(true), updF(true), updM(true), updRMV(true), support(NULL) {
  }

  void RigidBodyLink::updatehRef(const Vec &hParent, int j) {
    for(unsigned i=0; i<body.size(); i++) {
      RangeV I = RangeV(body[i]->gethInd(j),body[i]->gethInd(j)+body[i]->gethSize(j)-1);
      h[j][i]>>hParent(I);
      I = RangeV(support->gethInd(j),support->gethInd(j)+support->gethSize(j)-1);
      h[j][body.size()+i]>>hParent(I);
    }
  } 

  void RigidBodyLink::updaterRef(const Vec &rParent, int j) {
    for(unsigned i=0; i<body.size(); i++) {
      RangeV I = RangeV(body[i]->gethInd(j),body[i]->gethInd(j)+body[i]->gethSize(j)-1);
      r[j][i]>>rParent(I);
      I = RangeV(support->gethInd(j),support->gethInd(j)+support->gethSize(j)-1);
      r[j][body.size()+i]>>rParent(I);
    }
  } 

  void RigidBodyLink::updateWRef(const Mat &WParent, int j) {
    for(unsigned i=0; i<body.size(); i++) {
      RangeV J = RangeV(laInd,laInd+laSize-1);
      RangeV I = RangeV(body[i]->gethInd(j),body[i]->gethInd(j)+body[i]->gethSize(j)-1);

      W[j][i]>>WParent(I,J);
      I = RangeV(support->gethInd(j),support->gethInd(j)+support->gethSize(j)-1);
      W[j][body.size()+i]>>WParent(I,J);
    }
  } 

  void RigidBodyLink::updateVRef(const Mat &VParent, int j) {
    for(unsigned i=0; i<body.size(); i++) {
      RangeV J = RangeV(laInd,laInd+laSize-1);
      RangeV I = RangeV(body[i]->gethInd(j),body[i]->gethInd(j)+body[i]->gethSize(j)-1);

      V[j][i]>>VParent(I,J);
      I = RangeV(support->gethInd(j),support->gethInd(j)+support->gethSize(j)-1);
      V[j][body.size()+i]>>VParent(I,J);
    }
  } 

  void RigidBodyLink::updatePositions() {
    for(unsigned i=0; i<body.size(); i++) {
      C[i].setPosition(body[i]->getFrameForKinematics()->evalPosition());
      C[i].setOrientation(support->getOrientation());
    }
    updPos = false;
  }

  void RigidBodyLink::updateGeneralizedPositions() {
    rrel.init(0);
    for(unsigned i=0; i<body.size(); i++)
      rrel+=body[i]->evalqRel()*ratio[i];
    updrrel = false;
  }

  void RigidBodyLink::updateGeneralizedVelocities() {
    vrel.init(0);
    for(unsigned i=0; i<body.size(); i++)
      vrel+=body[i]->evaluRel()*ratio[i];
    updvrel = false;
  }

  void RigidBodyLink::updateForceDirections() {
    for(unsigned i=0; i<body.size(); i++) {
      DF[i] = support->evalOrientation()*body[i]->evalPJT();
      DM[i] = support->evalOrientation()*body[i]->evalPJR();
    }
    updFD = false;
  }

  void RigidBodyLink::updateForce() {
    for(unsigned i=0; i<body.size(); i++)
      F[i] = evalGlobalForceDirection(i)*evalGeneralizedForce()(iF)*ratio[i];
    updF = false;
  }

  void RigidBodyLink::updateMoment() {
    for(unsigned i=0; i<body.size(); i++)
      M[i] = evalGlobalMomentDirection(i)*evalGeneralizedForce()(iM)*ratio[i];
    updM = false;
  }

  void RigidBodyLink::updateR() {
    for(unsigned i=0; i<body.size(); i++) {
      RF[i].set(RangeV(0,2), RangeV(iF), evalGlobalForceDirection(i)*ratio[i]);
      RM[i].set(RangeV(0,2), RangeV(iM), evalGlobalMomentDirection(i)*ratio[i]);
    }
    updRMV = false;
  }

  void RigidBodyLink::updateh(int j) {
    if(j==0) {
      for(unsigned i=0; i<body.size(); i++)
        h[j][i]+=body[i]->evalJRel(j).T()*evalGeneralizedForce()*ratio[i];
    } else {
      for(unsigned i=0; i<body.size(); i++) {
        h[j][i]+=body[i]->getFrameForKinematics()->evalJacobianOfTranslation(j).T()*evalForce(i)  + body[i]->getFrameForKinematics()->evalJacobianOfRotation(j).T()*evalMoment(i);
        h[j][body.size()+i]-=C[i].evalJacobianOfTranslation(j).T()*evalForce(i) + C[i].evalJacobianOfRotation(j).T()*evalMoment(i);
      }
    }
  }

  void RigidBodyLink::updateW(int j) {
    if(j==0) {
      for(unsigned i=0; i<body.size(); i++)  {
        W[j][i]+=body[i]->evalJRel(j).T()*ratio[i];
      }
    } else {
      for(unsigned i=0; i<body.size(); i++) {
        W[j][i]+=body[i]->getFrameForKinematics()->evalJacobianOfTranslation(j).T()*evalRF(i) + body[i]->getFrameForKinematics()->evalJacobianOfRotation(j).T()*evalRM(i);
        W[j][body.size()+i]-=C[i].evalJacobianOfTranslation(j).T()*evalRF(i) + C[i].evalJacobianOfRotation(j).T()*evalRM(i);
      }
    }
  }

  void RigidBodyLink::updateg() {
    g = evalGeneralizedRelativePosition();
  }

  void RigidBodyLink::updategd() {
    gd = evalGeneralizedRelativeVelocity();
  }

  void RigidBodyLink::updatewb() {
    for(unsigned i=0; i<body.size(); i++)
      wb += body[i]->evaljRel()*ratio[i];
  }

  void RigidBodyLink::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_supportFrame!="")
        setSupportFrame(getByPath<Frame>(saved_supportFrame));
      MechanicalLink::init(stage);
    }
    else if(stage==resize) {
      MechanicalLink::init(stage);

      if(!support)
        support = body[0]->getFrameOfReference();

      F.resize(body.size());
      M.resize(body.size());
      iF = RangeV(0, body[0]->getPJT(0,false).cols()-1);
      iM = RangeV(0, body[0]->getPJR(0,false).cols()-1);
      rrel.resize(body[0]->getqRelSize());
      vrel.resize(body[0]->getuRelSize());
      if(isSetValued()) {
        g.resize(rrel.size());
        gd.resize(vrel.size());
        la.resize(vrel.size());
      }
      lambda.resize(vrel.size());
      for(unsigned int i=0; i<body.size(); i++) {
        DF.push_back(Mat3xV(iF.size()));
        DM.push_back(Mat3xV(iM.size()));
        RF.push_back(Mat3xV(vrel.size()));
        RM.push_back(Mat3xV(vrel.size()));
        h[0].push_back(Vec(body[i]->getFrameForKinematics()->gethSize()));
        h[1].push_back(Vec(6));
        W[0].push_back(Mat(body[i]->getFrameForKinematics()->gethSize(),laSize));
        W[1].push_back(Mat(6,laSize));
        stringstream s;
        s << "F" << i;
        C.push_back(FloatingRelativeFrame(s.str()));
        C[i].getJacobianOfTranslation(0,false).resize(support->gethSize());
        C[i].getJacobianOfRotation(0,false).resize(support->gethSize());
        C[i].getJacobianOfTranslation(1,false).resize(support->gethSize(1));
        C[i].getJacobianOfRotation(1,false).resize(support->gethSize(1));
        C[i].setParent(this);
        C[i].setFrameOfReference(support);
      }
      for(unsigned int i=0; i<body.size(); i++) {
        h[0].push_back(Vec(support->gethSize()));
        h[1].push_back(Vec(6));
        W[0].push_back(Mat(support->gethSize(),laSize));
        W[1].push_back(Mat(6,laSize));
        V[0].push_back(Mat(support->gethSize(),laSize));
        V[1].push_back(Mat(6,laSize));
      }
    }
    else if(stage==plotting) {
      updatePlotFeatures();
      if(getPlotFeature(plotRecursive)==enabled) {
        for(int i=0; i<(int)body.size(); i++)
          plotColumns.push_back("la("+numtostr(i)+")*ratio("+numtostr(i)+")");
        MechanicalLink::init(stage);
      }
    }
    else
      MechanicalLink::init(stage);
  }

  void RigidBodyLink::plot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      for(unsigned int i=0; i<body.size(); i++) {
        plotVector.push_back(ratio[i]*evalGeneralizedForce()(0));
      }
      MechanicalLink::plot();
    }
  }

  void RigidBodyLink::initializeUsingXML(DOMElement* element) {
    MechanicalLink::initializeUsingXML(element);
    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"supportFrame");
    if(e) saved_supportFrame=E(e)->getAttribute("ref");
  }

  void RigidBodyLink::resetUpToDate() {
    MechanicalLink::resetUpToDate();
    updPos = true;
    updVel = true;
    updFD = true;
    updF = true;
    updM = true;
    updRMV = true;
    for(unsigned int i=0; i<C.size(); i++)
      C[i].resetUpToDate();
  }

}
