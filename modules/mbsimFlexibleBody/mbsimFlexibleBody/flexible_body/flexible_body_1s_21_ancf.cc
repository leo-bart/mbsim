/* Copyright (C) 2004-2014 MBSim Development Team
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
 * Contact: thorsten.schindler@mytum.de
 */

#include <config.h>
#include <boost/swap.hpp>
#include "mbsimFlexibleBody/flexible_body/flexible_body_1s_21_ancf.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/eps.h"
#include "mbsim/environment.h"
#include "mbsim/utils/rotarymatrices.h"

using namespace fmatvec;
using namespace std;
using namespace MBSim;

namespace MBSimFlexibleBody {


  FlexibleBody1s21ANCF::FlexibleBody1s21ANCF(const string &name, bool openStructure_) : FlexibleBodyContinuum<double>(name), Elements(0), L(0), l0(0), E(0), A(0), I(0), rho(0), rc(0.), deps(0.), dkappa(0.), openStructure(openStructure_), initialised(false), v0(0.), Euler(false) {}

  void FlexibleBody1s21ANCF::GlobalVectorContribution(int n, const fmatvec::Vec& locVec, fmatvec::Vec& gloVec) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloVec(j,j+7) += locVec;
    }
    else { // ring closure at finite element (end,1)
      gloVec(j,j+3) += locVec(0,3);
      gloVec(0,  3) += locVec(4,7);
    }
  }

  void FlexibleBody1s21ANCF::GlobalMatrixContribution(int n, const fmatvec::Mat& locMat, fmatvec::Mat& gloMat) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloMat(Index(j,j+7),Index(j,j+7)) += locMat;
    }
    else { // ring closure at finite element (end,1)
      gloMat(Index(j,j+3),Index(j,j+3)) += locMat(Index(0,3),Index(0,3));
      gloMat(Index(j,j+3),Index(0,3)) += locMat(Index(0,3),Index(4,7));
      gloMat(Index(0,3),Index(j,j+3)) += locMat(Index(4,7),Index(0,3));
      gloMat(Index(0,3),Index(0,3)) += locMat(Index(4,7),Index(4,7));
    }
  }

  void FlexibleBody1s21ANCF::GlobalMatrixContribution(int n, const fmatvec::SymMat& locMat, fmatvec::SymMat& gloMat) {
    int j = 4 * n;

    if(n < Elements - 1 || openStructure==true) {
      gloMat(Index(j,j+7)) += locMat;
    }
    else { // ring closure at finite element (end,1) with angle difference 2*M_PI
      gloMat(Index(j,j+3))            += locMat(Index(0,3));
      gloMat(Index(j,j+3),Index(0,3)) += locMat(Index(0,3),Index(4,7));
      gloMat(Index(0,3))              += locMat(Index(4,7));
    }
  }

  void FlexibleBody1s21ANCF::updateKinematicsForFrame(ContourPointData &cp, FrameFeature ff, Frame *frame) {
    if(cp.getContourParameterType() == ContourPointData::continuum) { // frame on continuum
      double sLocal;
      int currentElement;
      BuildElement(cp.getLagrangeParameterPosition()(0), sLocal, currentElement); // Lagrange parameter of affected FE

      if(ff==position || ff==position_cosy || ff==all) {
        Vec tmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->computePosition(qElement[currentElement],ContourPointData(sLocal));
        cp.getFrameOfReference().setPosition(R->getPosition() + R->getOrientation() * tmp); // position
      }
      if(ff==firstTangent || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) {
        Vec tmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->tangent(qElement[currentElement],sLocal);
        cp.getFrameOfReference().getOrientation().set(1, R->getOrientation() * tmp); // tangent
      }
      if(ff==normal || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) {
        Vec tmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->tangent(qElement[currentElement],sLocal);
        tmp(1) *= -1.;
        boost::swap(tmp(0),tmp(1));
        cp.getFrameOfReference().getOrientation().set(0, R->getOrientation() * tmp); // normal
      }
      if(ff==secondTangent || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) cp.getFrameOfReference().getOrientation().set(2, -R->getOrientation().col(2)); // binormal 

      if(ff==velocity || ff==velocity_cosy || ff==velocities || ff==velocities_cosy || ff==all) {
        Vec tmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->computeVelocity(qElement[currentElement],uElement[currentElement],ContourPointData(sLocal));
        cp.getFrameOfReference().setVelocity(R->getOrientation() * tmp); //velocity
      }

      if(ff==angularVelocity || ff==velocities || ff==velocities_cosy || ff==all) {
        Vec tmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->computeAngularVelocity(qElement[currentElement],uElement[currentElement],ContourPointData(sLocal));
        cp.getFrameOfReference().setAngularVelocity(R->getOrientation() * tmp); // angular velocity
      }
    }
    else if(cp.getContourParameterType() == ContourPointData::node) { // frame on node
      const int &node = cp.getNodeNumber();

      Vec tmp(3,NONINIT);

      if(ff==position || ff==position_cosy || ff==all) {
        tmp(0) = q(4*node+0); tmp(1) = q(4*node+1); tmp(2) = 0.; 
        cp.getFrameOfReference().setPosition(R->getPosition() + R->getOrientation() * tmp); // position
      }

      if(ff==firstTangent || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) {
        tmp(0) =  q(4*node+2); tmp(1) = q(4*node+3); tmp(2) = 0.;
        tmp /= nrm2(tmp);
        cp.getFrameOfReference().getOrientation().set(1, R->getOrientation() * tmp); // tangent
      }
      if(ff==normal || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) {
        tmp(0) =  q(4*node+2); tmp(1) = -q(4*node+3); tmp(2) = 0.;
        tmp /= nrm2(tmp);
        boost::swap(tmp(0),tmp(1));
        cp.getFrameOfReference().getOrientation().set(0, R->getOrientation() * tmp); // normal
      }
      if(ff==secondTangent || ff==cosy || ff==position_cosy || ff==velocity_cosy || ff==velocities_cosy || ff==all) cp.getFrameOfReference().getOrientation().set(2, -R->getOrientation().col(2)); // binormal

      if(ff==velocity || ff==velocities || ff==velocity_cosy || ff==velocities_cosy || ff==all) {
        tmp(0) = u(4*node+0); tmp(1) = u(4*node+1); tmp(2) = 0.;

        if(Euler) {
          tmp(0) += v0*q(4*node+2);
          tmp(1) += v0*q(4*node+3);
        }
        cp.getFrameOfReference().setVelocity(R->getOrientation() * tmp); // velocity
      }

      if(ff==angularVelocity || ff==velocities || ff==velocities_cosy || ff==all) {
        if(Euler) {
          double der2_1; // curvature first component
          double der2_2; // curvature second component
          
          if(4*(node+1)+1 < qSize) {
            der2_1 = 6./(l0*l0)*(q(4*(node+1))-q(4*(node)))-2./l0*(q(4*(node+1)+2)+2.*q(4*(node)+2));
            der2_2 = 6./(l0*l0)*(q(4*(node+1)+1)-q(4*(node)+1))-2./l0*(q(4*(node+1)+3)+2.*q(4*(node)+3));
          }
          else {
            der2_1 = 6./(l0*l0)*(q(4*(node-1))-q(4*(node)))+2./l0*(q(4*(node-1)+2)+2.*q(4*(node)+2));
            der2_2 = 6./(l0*l0)*(q(4*(node-1)+1)-q(4*(node)+1))+2./l0*(q(4*(node-1)+3)+2.*q(4*(node)+3));
          }

          tmp(0) = 0.; tmp(1) = 0.; tmp(2) = (-q(4*node+3)*(u(4*node+2) + v0*der2_1)+q(4*node+2)*(u(4*node+3) + v0*der2_2))/sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3));
          cp.getFrameOfReference().setAngularVelocity(R->getOrientation() * tmp); // angular velocity
        }
        else {
          tmp(0) = 0.; tmp(1) = 0.; tmp(2) = (-q(4*node+3)*u(4*node+2)+q(4*node+2)*u(4*node+3))/sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3));
          cp.getFrameOfReference().setAngularVelocity(R->getOrientation() * tmp); // angular velocity
        }
      }
    }
    else throw MBSimError("ERROR(FlexibleBody1sANCF::updateKinematicsForFrame): ContourPointDataType should be 'ContourPointData::node' or 'ContourPointData::continuum'");

    if(frame!=0) { // frame should be linked to contour point data
      frame->setPosition       (cp.getFrameOfReference().getPosition());
      frame->setOrientation    (cp.getFrameOfReference().getOrientation());
      frame->setVelocity       (cp.getFrameOfReference().getVelocity());
      frame->setAngularVelocity(cp.getFrameOfReference().getAngularVelocity());
    }
  }

  void FlexibleBody1s21ANCF::updateJacobiansForFrame(ContourPointData &cp, Frame *frame) {
    Index All(0,3-1);
    Mat Jacobian(qSize,3,INIT,0.);

    if(cp.getContourParameterType() == ContourPointData::continuum) { // frame on continuum
      double sLocal;
      int currentElement;
      BuildElement(cp.getLagrangeParameterPosition()(0),sLocal,currentElement);
      Mat Jtmp = static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->JGeneralized(qElement[currentElement],sLocal);
      if(currentElement<Elements-1 || openStructure) {
        Jacobian(Index(4*currentElement,4*currentElement+7),All) = Jtmp;
      }
      else { // ringstructure
        Jacobian(Index(4*currentElement,4*currentElement+3),All) = Jtmp(Index(0,3),All);
        Jacobian(Index(               0,                 3),All) = Jtmp(Index(4,7),All);
      }
    }
    else if(cp.getContourParameterType() == ContourPointData::node) { // frame on node
      int node = cp.getNodeNumber();
      Jacobian(4*node,0) = 1.;
      Jacobian(4*node+1,1) = 1.;
      Jacobian(4*node+2,2) = -q(4*node+3);
      Jacobian(4*node+3,2) = q(4*node+2);
      Jacobian(Index(4*node+2,4*node+3),2) /= sqrt(q(4*node+2)*q(4*node+2)+q(4*node+3)*q(4*node+3));
    }
    else throw MBSimError("ERROR(FlexibleBody1s21ANCF::updateJacobiansForFrame): ContourPointDataType should be 'ContourPointData::node' or 'ContourPointData::continuum'");

    cp.getFrameOfReference().setJacobianOfTranslation(R->getOrientation()(Index(0,2),Index(0,1))*Jacobian(Index(0,qSize-1),Index(0,1)).T());
    cp.getFrameOfReference().setJacobianOfRotation   (R->getOrientation()(Index(0,2),Index(2,2))*Jacobian(Index(0,qSize-1),Index(2,2)).T());

    // cp.getFrameOfReference().setGyroscopicAccelerationOfTranslation(TODO)
    // cp.getFrameOfReference().setGyroscopicAccelerationOfRotation(TODO)

    if(frame!=0) { // frame should be linked to contour point data
      frame->setJacobianOfTranslation(cp.getFrameOfReference().getJacobianOfTranslation());
      frame->setJacobianOfRotation   (cp.getFrameOfReference().getJacobianOfRotation());
      frame->setGyroscopicAccelerationOfTranslation(cp.getFrameOfReference().getGyroscopicAccelerationOfTranslation());
      frame->setGyroscopicAccelerationOfRotation   (cp.getFrameOfReference().getGyroscopicAccelerationOfRotation());
    }
  }

  void FlexibleBody1s21ANCF::init(InitStage stage) {
    if(stage==unknownStage) {
      FlexibleBodyContinuum<double>::init(stage);

      initialised = true;

      l0 = L/Elements;
      Vec g = R->getOrientation()(Index(0,2),Index(0,1)).T()*MBSimEnvironment::getInstance()->getAccelerationOfGravity();

      for(int i=0;i<Elements;i++) {
        qElement.push_back(Vec(8,INIT,0.));
        uElement.push_back(Vec(8,INIT,0.));
        discretization.push_back(new FiniteElement1s21ANCF(l0, A*rho, E*A, E*I, g, Euler, v0));
        if(fabs(rc) > epsroot())
          static_cast<FiniteElement1s21ANCF*>(discretization[i])->setCurlRadius(rc);
        static_cast<FiniteElement1s21ANCF*>(discretization[i])->setMaterialDamping(deps,dkappa);
      }
      initM();
    }
    else if(stage==MBSim::plot) {
		  for(int i=0;i<q.size()/4;i++) {
		    plotColumns.push_back("vel_abs node ("+numtostr(i)+")");
		  }

#ifdef HAVE_OPENMBVCPPINTERFACE
      ((OpenMBV::SpineExtrusion*)openMBVBody)->setInitialRotation(AIK2Cardan(R->getOrientation()));
#endif
      FlexibleBodyContinuum<double>::init(stage);
    }
    else
      FlexibleBodyContinuum<double>::init(stage);
  }

  void FlexibleBody1s21ANCF::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled && openMBVBody) {
        vector<double> data;
        data.push_back(t);
        double ds = openStructure ? L/(((OpenMBV::SpineExtrusion*)openMBVBody)->getNumberOfSpinePoints()-1) : L/(((OpenMBV::SpineExtrusion*)openMBVBody)->getNumberOfSpinePoints()-2);

        for(int i=0; i<((OpenMBV::SpineExtrusion*)openMBVBody)->getNumberOfSpinePoints(); i++) {
          Vec  X(6,NONINIT);

          if(Euler) {
            X = computeState(ds*i+v0*t); // expects Euler coordinate
          }
          else {  
            X = computeState(ds*i); // expects Lagrange coordinate
          }

          Vec tmp(3,NONINIT); tmp(0) = X(0); tmp(1) = X(1); tmp(2) = 0.; // temporary vector used for compensating planar description
          Vec pos = R->getPosition() + R->getOrientation() * tmp;
          data.push_back(pos(0)); // global x-position
          data.push_back(pos(1)); // global y-position
          data.push_back(pos(2)); // global z-position
          data.push_back(0.); // local twist
        }

        ((OpenMBV::SpineExtrusion*)openMBVBody)->append(data);
      }
#endif
    }

    if(Euler) {
		  for(int i=0;i<q.size()/4;i++) {
		  	plotVector.push_back(sqrt(pow(u(4*i+0) + v0*q(4*i+2),2.)+pow(u(4*i+1) + v0*q(4*i+3),2.)));
		  }
    }
    else {
		  for(int i=0;i<q.size()/4;i++) {
		  	plotVector.push_back(sqrt(pow(u(4*i+0),2.)+pow(u(4*i+1),2.)));
		  }      
    }

    FlexibleBodyContinuum<double>::plot(t,dt);
  }

  void FlexibleBody1s21ANCF::setNumberElements(int n){
    Elements = n;
    if(openStructure) {
      qSize = 4 * (n+1);
    } else {
      qSize = 4 *  n   ;
    }
    uSize[0] = qSize;
    uSize[1] = qSize;
    q0.resize(qSize);
    u0.resize(uSize[0]);
  }

  Vec FlexibleBody1s21ANCF::computeState(double sGlobal) {
    double sLocal;
    int currentElement;
    BuildElement(sGlobal, sLocal, currentElement); // Lagrange/Euler parameter of affected FE
    return static_cast<FiniteElement1s21ANCF*>(discretization[currentElement])->StateBalken(qElement[currentElement],uElement[currentElement],sLocal);
  }

  void FlexibleBody1s21ANCF::BuildElements() {
    for(int i=0;i<Elements;i++) {
      int n = 4 * i ;

      if(i<Elements-1 || openStructure==true) {
        qElement[i] << q(n,n+7);
        uElement[i] << u(n,n+7);
      }
      else { // last finite element and ring closure
        qElement[i](0,3) << q(n,n+3);
        uElement[i](0,3) << u(n,n+3);
        qElement[i](4,7) << q(0,3);
        uElement[i](4,7) << u(0,3);
      }
    }
  }

  void FlexibleBody1s21ANCF::BuildElement(const double& sGlobal, double& sLocal, int& currentElement) {
    double remainder = fmod(sGlobal,L);
    if(openStructure && sGlobal >= L) remainder += L; // remainder \in (-eps,L+eps)
    if(!openStructure && sGlobal < 0.) remainder += L; // remainder \in [0,L)

    currentElement = int(remainder/l0);
    sLocal = remainder - (currentElement) * l0; // Lagrange/Euler parameter of the affected FE with sLocal==0 in the middle of the FE and sGlobal==0 at the beginning of the beam

    // contact solver computes too large sGlobal at the end of the entire beam is not considered only for open structure
    // for closed structure even sGlobal < L (but sGlobal ~ L) values could lead - due to numerical problems - to a wrong currentElement computation
    if(currentElement >= Elements) {
      currentElement =  Elements-1;
      sLocal += l0;
    }
  }

  void FlexibleBody1s21ANCF::initM() {
    for (int i = 0; i < (int) discretization.size(); i++)
      static_cast<FiniteElement1s21ANCF*>(discretization[i])->initM(); // compute attributes of finite element
    for (int i = 0; i < (int) discretization.size(); i++)
      GlobalMatrixContribution(i, discretization[i]->getM(), M[0]); // assemble
    for (int i = 0; i < (int) discretization.size(); i++) {
      int j = 4 * i;
      LLM[0](Index(j, j + 3)) = facLL(M[0](Index(j, j + 3)));
      if (openStructure && i == (int) discretization.size() - 1)
        LLM[0](Index(j + 4, j + 7)) = facLL(M[0](Index(j + 4, j + 7)));
    }
  }

  void FlexibleBody1s21ANCF::initInfo() {
    FlexibleBodyContinuum<double>::init(unknownStage);
    l0 = L/Elements;
    Vec g = Vec("[0.;0.;0.]");
    for(int i=0;i<Elements;i++) {
      discretization.push_back(new FiniteElement1s21ANCF(l0, A*rho, E*A, E*I, g, Euler, v0));
      qElement.push_back(Vec(discretization[0]->getqSize(),INIT,0.));
      uElement.push_back(Vec(discretization[0]->getuSize(),INIT,0.));
    }
    BuildElements();
  }

  void FlexibleBody1s21ANCF::initRelaxed(double alpha) {
    if(!initialised) {
      if(Elements==0)
        throw(new MBSimError("ERROR (FlexibleBody1s21ANCF::initRelaxed): Set number of finite elements!"));
      Vec q0Dummy(q0.size(),INIT,0.);
      if(openStructure) {
        Vec direction(2);
        direction(0) = cos(alpha);
        direction(1) = sin(alpha);

        for(int i=0;i<=Elements;i++) {
          q0Dummy(4*i+0,4*i+1) = direction*double(L/Elements*i);
          q0Dummy(4*i+2) = direction(0);
          q0Dummy(4*i+3) = direction(1);
        }
      }
      else {
        double R = L/(2*M_PI);

        for(int i=0;i<Elements;i++) {
          double alpha_ = i*(2*M_PI)/Elements;
          q0Dummy(4*i+0) = R*cos(alpha_);
          q0Dummy(4*i+1) = R*sin(alpha_);
          q0Dummy(4*i+2) = -sin(alpha_);
          q0Dummy(4*i+3) = cos(alpha_);
        }
      }
      setq0(q0Dummy);
      setu0(Vec(q0Dummy.size(),INIT,0.));
    }
  }

}

