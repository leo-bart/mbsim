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
 * Contact: martin.o.foerg@googlemail.com
 */

#include <config.h>
#include "flexible_body_ffr.h"
#include "mbsimFlexibleBody/frames/fixed_nodal_frame.h"
#include "mbsim/contours/rigid_contour.h"
#include "mbsim/dynamic_system.h"
#include "mbsim/links/joint.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/objectfactory.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematics/rotation_about_axes_xyz.h"
#include "mbsim/functions/kinematics/rotation_about_axes_zxz.h"
#include "mbsim/functions/kinematics/rotation_about_axes_zyx.h"
#include "mbsim/functions/kinematics/rotation_about_axes_xyz_mapping.h"
#include "mbsim/functions/kinematics/rotation_about_axes_zxz_mapping.h"
#include "mbsim/functions/kinematics/rotation_about_axes_zyx_mapping.h"
#include "mbsim/functions/kinematics/rotation_about_axes_xyz_transformed.h"
#include "mbsim/functions/kinematics/rotation_about_axes_xyz_transformed_mapping.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/rigidbody.h>
#include <openmbvcppinterface/invisiblebody.h>
#include <openmbvcppinterface/objectfactory.h>
#include <openmbvcppinterface/group.h>
#endif

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  Range<Var,Var> i02(0,2);

//  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(FlexibleBodyFFR, MBSIMFLEXIBLEBODY%"FlexibleBodyFFR")

  FlexibleBodyFFR::FlexibleBodyFFR(const string &name) : Body(name), m(0), ne(0), coordinateTransformation(true), APK(EYE), fTR(0), fPrPK(0), fAPK(0), frameForJacobianOfRotation(0), translationDependentRotation(false), constJT(false), constJR(false), constjT(false), constjR(false), updPjb(true), updGC(true), updT(true), updMb(true) {

    updKJ[0] = true;
    updKJ[1] = true;

    K=new FrameFFR("K");
    Body::addFrame(K);
#ifdef HAVE_OPENMBVCPPINTERFACE
    openMBVFrame=K;
#endif

    updateJacobians_[0] = &FlexibleBodyFFR::updateJacobians0;
    updateJacobians_[1] = &FlexibleBodyFFR::updateJacobians1;
    updateKJ_[0] = &FlexibleBodyFFR::updateKJ0;
    updateKJ_[1] = &FlexibleBodyFFR::updateKJ1;
  }

  FlexibleBodyFFR::~FlexibleBodyFFR() {
    if(fPrPK) { delete fPrPK; fPrPK=0; }
    if(fAPK) { delete fAPK; fAPK=0; }
    if(fTR) { delete fTR; fTR=0; }
  }

  void FlexibleBodyFFR::updateh(int index) {
    h[index] += evalKJ(index).T()*(evalhb() - evalMb()*evalKi());
  }

  void FlexibleBodyFFR::calcqSize() {
    Body::calcqSize();
    qSize = nq;
  }

  void FlexibleBodyFFR::calcuSize(int j) {
    Body::calcuSize(j);
    if(j==0)
      uSize[j] = nu[j];
    else
      uSize[j] = 6 + ne;
  }

  void FlexibleBodyFFR::determineSID() {
    Cr0.resize(ne,NONINIT);
    if(not(PPdm.size()))
      PPdm.resize(3,vector<SqrMatV>(3));
    Gr0.resize(ne);
    Gr1.resize(ne);
    Ge.resize(ne);
    Oe0.resize(ne,NONINIT);
    Oe1.resize(6);
    std::vector<std::vector<fmatvec::SqrMatV> > Kom(3);
    mmi0(0,0) = rrdm(1,1) + rrdm(2,2);
    mmi0(0,1) = -rrdm(1,0);
    mmi0(0,2) = -rrdm(2,0);
    mmi0(1,1) = rrdm(0,0) + rrdm(2,2);
    mmi0(1,2) = -rrdm(2,1);
    mmi0(2,2) = rrdm(0,0) + rrdm(1,1);
    for(int i=0; i<3; i++) {
      Kom[i].resize(3);
      for(int j=0; j<3; j++) {
        Kom[i][j].resize(ne,NONINIT);
        if(i!=j)
          Kom[i][j] = PPdm[i][j];
      }
    }
    Kom[0][0] = -PPdm[1][1]-PPdm[2][2];
    Kom[1][1] = -PPdm[2][2]-PPdm[0][0];
    Kom[2][2] = -PPdm[0][0]-PPdm[1][1];

    Me.resize(ne,NONINIT);
    mmi1.resize(ne);
    mmi2.resize(ne);

    for(int i=0; i<ne; i++) {
      Gr1[i].resize(ne);
      mmi2[i].resize(ne);
    }

    for(int i=0; i<ne; i++) {
      mmi1[i](0,0) = 2.*(rPdm[1][1](i) + rPdm[2][2](i));
      mmi1[i](0,1) = -(rPdm[1][0](i) + rPdm[0][1](i));
      mmi1[i](0,2) = -(rPdm[2][0](i) + rPdm[0][2](i));
      mmi1[i](1,1) = 2.*(rPdm[0][0](i) + rPdm[2][2](i));
      mmi1[i](1,2) = -(rPdm[2][1](i) + rPdm[1][2](i));
      mmi1[i](2,2) = 2.*(rPdm[0][0](i) + rPdm[1][1](i));

      Oe0.e(i,0) = -rPdm[2][2](i) - rPdm[1][1](i);
      Oe0.e(i,1) = -rPdm[2][2](i) - rPdm[0][0](i);
      Oe0.e(i,2) = -rPdm[1][1](i) - rPdm[0][0](i);
      Oe0(i,3) = rPdm[0][1](i) + rPdm[1][0](i);
      Oe0(i,4) = rPdm[1][2](i) + rPdm[2][1](i);
      Oe0(i,5) = rPdm[2][0](i) + rPdm[0][2](i);

      Gr0[i](0,0) = 2.*(rPdm[2][2](i) + rPdm[1][1](i));
      Gr0[i](0,1) = -2.*rPdm[1][0](i);
      Gr0[i](0,2) = -2.*rPdm[2][0](i);
      Gr0[i](1,0) = -2.*rPdm[0][1](i);
      Gr0[i](1,1) = 2.*(rPdm[2][2](i) + rPdm[0][0](i));
      Gr0[i](1,2) = -2.*rPdm[2][1](i);
      Gr0[i](2,0) = -2.*rPdm[0][2](i);
      Gr0[i](2,1) = -2.*rPdm[1][2](i);
      Gr0[i](2,2) = 2.*(rPdm[1][1](i) + rPdm[0][0](i));

      Cr0.e(i,0) = rPdm[1][2].e(i) - rPdm[2][1].e(i);
      Cr0.e(i,1) = rPdm[2][0].e(i) - rPdm[0][2].e(i);
      Cr0.e(i,2) = rPdm[0][1].e(i) - rPdm[1][0].e(i);

      for(int j=i; j<ne; j++) {
        Me.ej(i,j) = PPdm[0][0].e(i,j) + PPdm[1][1].e(i,j) + PPdm[2][2].e(i,j);

        mmi2[i][j].e(0,0) = PPdm[1][1].e(i,j) + PPdm[2][2].e(i,j);
        mmi2[i][j].e(1,1) = PPdm[0][0].e(i,j) + PPdm[2][2].e(i,j);
        mmi2[i][j].e(2,2) = PPdm[0][0].e(i,j) + PPdm[1][1].e(i,j);
        mmi2[i][j].e(0,1) = -PPdm[1][0].e(i,j);
        mmi2[i][j].e(0,2) = -PPdm[2][0].e(i,j);
        mmi2[i][j].e(1,2) = -PPdm[2][1].e(i,j);
        mmi2[i][j].e(1,0) = -PPdm[0][1].e(i,j);
        mmi2[i][j].e(2,0) = -PPdm[0][2].e(i,j);
        mmi2[i][j].e(2,1) = -PPdm[1][2].e(i,j);
        mmi2[j][i] = mmi2[i][j].T();

        Gr1[i][j] = 2.*mmi2[i][j];
        Gr1[j][i] = Gr1[i][j].T();
      }
    }
    Ct0 = Pdm.T();
    for(unsigned int i=0; i<K0t.size(); i++)
      Ct1.push_back(K0t[i]);

    std::vector<fmatvec::SqrMatV> Kr(3);
    for(int i=0; i<3; i++)
      Kr[i].resize(ne,NONINIT);
    Kr[0] = -PPdm[1][2] + PPdm[1][2].T();
    Kr[1] = -PPdm[2][0] + PPdm[2][0].T();
    Kr[2] = -PPdm[0][1] + PPdm[0][1].T();

    for(unsigned int i=0; i<Kr.size(); i++)
      Cr1.push_back(Kr[i]);
    for(unsigned int i=0; i<K0r.size(); i++)
      Cr1[i] += K0r[i];

    Ge.resize(3);
    for(int i=0; i<3; i++)
      Ge[i].resize() = 2.*Kr[i];

    for(int i=0; i<3; i++)
      Oe1[i].resize() = Kom[i][i];
    Oe1[3].resize() = Kom[0][1] + Kom[0][1].T();
    Oe1[4].resize() = Kom[1][2] + Kom[1][2].T();
    Oe1[5].resize() = Kom[2][0] + Kom[2][0].T();
    for(unsigned int i=0; i<K0om.size(); i++)
      Oe1[i] += K0om[i];

    if(not(De0.size()))
      De0 = beta.e(0)*Me + beta.e(1)*Ke0;

    if(Knl1.size()) {
      Ke1.resize(Knl1.size());
      if(Knl2.size()) {
        Ke2.resize(Knl2.size());
        for(unsigned int i=0; i<Knl2.size(); i++)
          Ke2[i].resize(Knl2.size());
      }
      for(unsigned int i=0; i<Knl1.size(); i++) {
        Ke1[i].resize() = (Knl1[i].T() + 0.5*Knl1[i]);
        for(unsigned int j=0; j<Knl2.size(); j++)
          Ke2[i][j].resize() = 0.5*Knl2[i][j];
      }
    }
 }

  void FlexibleBodyFFR::prefillMassMatrix() {
    M_.resize(6+ne,NONINIT);
    for(int i=0; i<3; i++) {
      M_.ej(i,i)=m;
      for(int j=i+1; j<3; j++)
        M_.ej(i,j)=0;
    }
    for(int i=0; i<ne; i++) {
      for(int j=0; j<3; j++)
        M_.e(i+6,j) = Ct0.e(i,j);
      for(int j=i; j<ne; j++)
        M_.ej(i+6,j+6) = Me.ej(i,j);
    }
  }

  void FlexibleBodyFFR::init(InitStage stage) {
    if(stage==preInit) {
      for(unsigned int k=1; k<frame.size(); k++) {
        if(not(static_cast<FixedNodalFrame*>(frame[k])->getFrameOfReference()))
          static_cast<FixedNodalFrame*>(frame[k])->setFrameOfReference(K);
      }

      ne = Pdm.cols()?Pdm.cols():Me.size();
      for(unsigned int i=1; i<frame.size(); i++)
        static_cast<FixedNodalFrame*>(frame[i])->setNumberOfModeShapes(ne);

      for(unsigned int k=0; k<contour.size(); k++) {
        RigidContour *contour_ = dynamic_cast<RigidContour*>(contour[k]);
        if(contour_ and not(contour_->getFrameOfReference()))
          contour_->setFrameOfReference(K);
      }

      Body::init(stage);

      int nqT=0, nqR=0, nuT=0, nuR=0;
      if(fPrPK) {
        nqT = fPrPK->getArg1Size();
        nuT = fPrPK->getArg1Size(); // TODO fTT->getArg1Size()
      }
      if(fAPK) {
        nqR = fAPK->getArg1Size();
        nuR = fAPK->getArg1Size(); // TODO fTR->getArg1Size()
      }
 
      if(translationDependentRotation) {
        assert(nqT == nqR);
        assert(nuT == nuR);
        nq = nqT + ne;
        nu[0] = nuT + ne;
        iqT = Range<Var,Var>(0,nqT+nqR-1);
        iqR = Range<Var,Var>(0,nqT+nqR-1);
        iqE = Range<Var,Var>(nqT+nqR,nq-1);
        iuT = Range<Var,Var>(0,nuT+nuR-1);
        iuR = Range<Var,Var>(0,nuT+nuR-1);
        iuE = Range<Var,Var>(nuT+nuR,nu[0]-1);
      }
      else {
        nq = nqT + nqR + ne;
        nu[0] = nuT + nuR + ne;
        iqT = Range<Var,Var>(0,nqT-1);
        iqR = Range<Var,Var>(nqT,nqT+nqR-1);
        iqE = Range<Var,Var>(nqT+nqR,nq-1);
        iuT = Range<Var,Var>(0,nuT-1);
        iuR = Range<Var,Var>(nuT,nqT+nqR-1);
        iuE = Range<Var,Var>(nuT+nuR,nu[0]-1);
      }

      nu[1] = 6 + ne;
    }
    else if(stage==resize) {
      Body::init(stage);

  
      KJ[0].resize(6+ne,hSize[0]);
      KJ[1].resize(6+ne,hSize[1]);
      K->getJacobianOfDeformation(0,false).resize(ne,hSize[0]);
      K->getJacobianOfDeformation(1,false).resize(ne,hSize[1]);

      for(int i=0; i<ne; i++) {
        KJ[0](6+i,hSize[0]-ne+i) = 1;
        KJ[1](6+i,hSize[1]-ne+i) = 1;
        K->getJacobianOfDeformation(0,false)(i,hSize[0]-ne+i) = 1;
        K->getJacobianOfDeformation(1,false)(i,hSize[1]-ne+i) = 1;
      }

      Ki.resize(6+ne);

      PJT[0].resize(nu[0]);
      PJR[0].resize(nu[0]);

      PJT[1].resize(nu[1]);
      PJR[1].resize(nu[1]);
      for(int i=0; i<3; i++)
	PJT[1](i,i) = 1;
      for(int i=3; i<6; i++)
	PJR[1](i-3,i) = 1;

      qRel.resize(nq);
      uRel.resize(nu[0]);
      q.resize(qSize);
      u.resize(uSize[0]);

      TRel.resize(nq,nu[0],Eye());

      WJTrel.resize(nu[0]);
      WJRrel.resize(nu[0]);

      updateM_ = &FlexibleBodyFFR::updateMNotConst;
      updateLLM_ = &FlexibleBodyFFR::updateLLMNotConst;
    }
    else if(stage==unknownStage) {
      Body::init(stage);

      if(Me.size()==0)
        determineSID();
      prefillMassMatrix();

      K->getJacobianOfTranslation(1,false) = PJT[1];
      K->getJacobianOfRotation(1,false) = PJR[1];

      bool cb = false;
      StateDependentFunction<RotMat3> *Atmp = dynamic_cast<StateDependentFunction<RotMat3>*>(fAPK);
      if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesXYZ<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesXYZMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }
      else if(Atmp and dynamic_cast<RotationAboutAxesXYZTransformed<VecV>*>(Atmp->getFunction())) {
        cb = true;
        if(coordinateTransformation) {
          fTR = new RotationAboutAxesXYZTransformedMapping<VecV>;
          fTR->setParent(this);
          constJR = true;
          constjR = true;
          PJRR = SqrMat3(EYE);
          PJR[0].set(i02,iuR,PJRR);
        }
      }
      else if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesZXZ<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesZXZMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }
      else if(Atmp and coordinateTransformation and dynamic_cast<RotationAboutAxesZYX<VecV>*>(Atmp->getFunction())) {
        fTR = new RotationAboutAxesZYXMapping<VecV>;
        fTR->setParent(this);
        constJR = true;
        constjR = true;
        PJRR = SqrMat3(EYE);
        PJR[0].set(i02,iuR,PJRR);
      }

      if(fPrPK) {
        if(fPrPK->constParDer1()) {
          constJT = true;
          PJTT = fPrPK->parDer1(qTRel,0);
          PJT[0].set(i02,iuT,PJTT);
        }
        if(fPrPK->constParDer2()) {
          constjT = true;
          PjhT = fPrPK->parDer2(qTRel,0);
        }
      }
      if(fAPK) {
        if(fAPK->constParDer1()) {
          constJR = true;
          PJRR = fTR?fAPK->parDer1(qRRel,0)*(*fTR)(qRRel):fAPK->parDer1(qRRel,0);
          PJR[0].set(i02,iuR,PJRR);
        }
        if(fAPK->constParDer2()) {
          constjR = true;
          PjhR = fAPK->parDer2(qRRel,0);
        }
      }

      if(cb) {
        frameForJacobianOfRotation = K;
        // TODO: do not invert generalized mass matrix in case of special
        // parametrisation
//        if(K == C && dynamic_cast<DynamicSystem*>(R->getParent())) {
//          if(fPrPK) {
//            fPrPK->updateJacobian(qRel(iqT),0);
//            PJT[0].set(i02,iuT,fPrPK->getJacobian());
//          }
//          if(fAPK) {
//            fAPK->updateJacobian(qRel(iqR),0);
//            PJR[0].set(i02,iuR,fAPK->getJacobian());
//          }
//          updateM_ = &FlexibleBodyFFR::updateMConst;
//          Mbuf = m*JTJ(PJT[0]) + JTMJ(SThetaS,PJR[0]);
//          LLM[0] = facLL(Mbuf);
//          facLLM_ = &FlexibleBodyFFR::facLLMConst;
//        }
      }
      else
        frameForJacobianOfRotation = R;

      T.init(Eye());
    }
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(notMinimalState)==enabled) {
          for(int i=0; i<nq; i++)
            plotColumns.push_back("qRel("+numtostr(i)+")");
          for(int i=0; i<nu[0]; i++)
            plotColumns.push_back("uRel("+numtostr(i)+")");
        }
        Body::init(stage);
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled) {
          if(getPlotFeature(openMBV)==enabled && FWeight) {
            FWeight->setName("Weight");
            openMBVGrp->addObject(FWeight);
          }
        }
#endif
      }
    }
    else
      Body::init(stage);
    if(fTR) fTR->init(stage);
    if(fPrPK) fPrPK->init(stage);
    if(fAPK) fAPK->init(stage);
  }

  void FlexibleBodyFFR::initz() {
    Object::initz();
    qRel>>q;
    uRel>>u;
    TRel>>T;
  }

  void FlexibleBodyFFR::setUpInverseKinetics() {
    InverseKineticsJoint *joint = new InverseKineticsJoint(string("Joint_")+R->getParent()->getName()+"_"+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(joint);
    joint->setForceDirection(Mat3xV(3,EYE));
    joint->setMomentDirection(Mat3xV(3,EYE));
    joint->connect(R,K);
    joint->setBody(this);
    if(FArrow)
      joint->setOpenMBVForce(FArrow);
    if(MArrow)
      joint->setOpenMBVMoment(MArrow);
  }

  void FlexibleBodyFFR::plot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(notMinimalState)==enabled) {
        for(int i=0; i<nq; i++)
          plotVector.push_back(qRel.e(i));
        for(int i=0; i<nu[0]; i++)
          plotVector.push_back(uRel.e(i));
      }

#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled) {
        if(FWeight) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 WrOP=K->evalPosition();
          Vec3 WG = m*MBSimEnvironment::getInstance()->getAccelerationOfGravity();
          data.push_back(WrOP(0));
          data.push_back(WrOP(1));
          data.push_back(WrOP(2));
          data.push_back(WG(0));
          data.push_back(WG(1));
          data.push_back(WG(2));
          data.push_back(1.0);
          FWeight->append(data);
        }
        if(openMBVBody) {
          vector<double> data;
          data.push_back(getTime());
          Vec3 WrOS=openMBVFrame->evalPosition();
          Vec3 cardan=AIK2Cardan(openMBVFrame->getOrientation());
          data.push_back(WrOS(0));
          data.push_back(WrOS(1));
          data.push_back(WrOS(2));
          data.push_back(cardan(0));
          data.push_back(cardan(1));
          data.push_back(cardan(2));
          data.push_back(0);
          std::dynamic_pointer_cast<OpenMBV::RigidBody>(openMBVBody)->append(data);
        }
      }
#endif
      Body::plot();
    }
  }

  void FlexibleBodyFFR::updateMb() {
    Vec3 mc = rdm + Pdm*q(iqE);
    SqrMat3 mtc = tilde(mc);

    SymMat3 I1;
    SqrMat3 I2;
    for (int i=0; i<ne; i++) {
      I1 += mmi1[i]*q(iqE).e(i);
      for (int j=0; j<ne; j++)
        I2 += mmi2[i][j]*(q(iqE).e(i)*q(iqE).e(j));
    }

    SymMat3 I = mmi0 + I1 + SymMat3(I2);
    for(int i=0; i<3; i++) {
      for(int j=0; j<3; j++) {
        M_.e(i+3,j+3) = I.e(i,j);
        M_.e(i+3,j) = mtc.e(i,j);
      }
    }

    MatVx3 Cr1_(ne,NONINIT);
    for(int i=0; i<3; i++)
      Cr1_.set(i,Cr1[i]*q(iqE));
    for(int i=0; i<ne; i++)
      for(int j=0; j<3; j++)
        M_.e(i+6,j+3) = Cr0.e(i,j) + Cr1_.e(i,j);

    MatVx3 Ct_ = Ct0;
    if(Ct1.size()) {
      MatVx3 Ct1_(ne,NONINIT);
      for(int i=0; i<3; i++)
        Ct1_.set(i,Ct1[i]*q(iqE));
      Ct_ += Ct1_;
      for(int i=0; i<ne; i++)
        for(int j=0; j<3; j++)
          M_.e(i+6,j) = Ct_.e(i,j);
    }

    fmatvec::Matrix<fmatvec::General,fmatvec::Var,fmatvec::Fixed<6>,double> Oe_(ne,NONINIT), Oe1_(ne,NONINIT);
    for(int i=0; i<6; i++)
      Oe1_.set(i,Oe1[i]*q(iqE));

    Oe_ = Oe0 + Oe1_;

    std::vector<fmatvec::SqrMat3> Gr1_(ne);
    fmatvec::SqrMat3 hom21;
    for(int i=0; i<ne; i++) {
      Gr1_[i].init(0);
      for(int j=0; j<ne; j++)
        Gr1_[i] += Gr1[j][i]*q(iqE).e(j);
      hom21 += (Gr0[i]+Gr1_[i])*u(iuE).e(i);
    }
    fmatvec::MatVx3 Ge_(ne,NONINIT);
    for(int i=0; i<3; i++)
      Ge_.set(i,Ge[i]*u(iuE));

    Vec3 om = K->evalOrientation().T()*K->evalAngularVelocity();
    Vector<Fixed<6>,double> omq;
    for(int i=0; i<3; i++)
      omq.e(i) = pow(om.e(i),2);
    omq.e(3) = om.e(0)*om.e(1); 
    omq.e(4) = om.e(1)*om.e(2); 
    omq.e(5) = om.e(0)*om.e(2);

    VecV hom(6+ne), hg(6+ne), he(6+ne);

    hom.set(Index(0,2),-crossProduct(om,crossProduct(om,mc))-2.*crossProduct(om,Ct_.T()*u(iuE)));
    hom.set(Index(3,5),-(hom21*om) - crossProduct(om,I*om));
    hom.set(Index(6,6+ne-1),-(Ge_*om) - Oe_*omq);

    Vec Kg = K->getOrientation().T()*(MBSimEnvironment::getInstance()->getAccelerationOfGravity());
    hg.set(Index(0,2),m*Kg);
    hg.set(Index(3,5),mtc*Kg);
    hg.set(Index(6,hg.size()-1),Ct_*Kg);

    SqrMatV Ke_ = SqrMatV(Ke0);
    for(unsigned int i=0; i<Ke1.size(); i++) {
      Ke_ += Ke1[i]*q(iqE).e(i);
      for(unsigned int j=0; j<Ke2.size(); j++)
        Ke_ += Ke2[i][j]*(q(iqE).e(i)*q(iqE).e(j));
    }

    VecV ke = Ke_*q(iqE) + De0*qd(iqE);

    if(ksigma0.size())
      ke += ksigma0;
    if(ksigma1.size())
      ke += ksigma1*q(iqE);

    he.set(Index(6,hg.size()-1),ke);

    h_ = hom + hg - he;

    updMb = false;
  }

  void FlexibleBodyFFR::updateqd() {
    qd(iqT) = uRel(iuT);
    if(fTR)
      qd(iqR) = (*fTR)(qRel(iuR))*uRel(iuR);
    else
      qd(iqR) = uRel(iuR);
    qd(iqE) = uRel(iuE);
  }

  void FlexibleBodyFFR::updatedq() {
    dq(iqT) = uRel(iuT)*getStepSize();
    if(fTR)
      dq(iqR) = (*fTR)(qRel(iuR))*uRel(iuR)*getStepSize();
    else
      dq(iqR) = uRel(iuR)*getStepSize();
    dq(iqE) = uRel(iuE)*getStepSize();
  }
  void FlexibleBodyFFR::updateT() {
    if(fTR) TRel(iqR,iuR) = (*fTR)(qRel(iuR));
  }

  void FlexibleBodyFFR::updateGeneralizedCoordinates() {
    qTRel = qRel(iqT);
    qRRel = qRel(iqR);
    uTRel = uRel(iuT);
    uRRel = uRel(iuR);
    updGC = false;
  }

 void FlexibleBodyFFR::updatePositions() {
    if(fPrPK) PrPK = (*fPrPK)(evalqTRel(),getTime());
    if(fAPK) APK = (*fAPK)(evalqRRel(),getTime());
    WrPK = R->evalOrientation()*PrPK;
    updPos = false;
  }

  void FlexibleBodyFFR::updateVelocities() {
    if(fPrPK) WvPKrel = R->evalOrientation()*(evalPJTT()*evaluTRel() + PjhT);
    if(fAPK) WomPK = frameForJacobianOfRotation->evalOrientation()*(evalPJRR()*evaluRRel() + PjhR);
    updVel = false;
  }

  void FlexibleBodyFFR::updateJacobians() {
    if(fPrPK) {
      if(!constJT) {
        PJTT = fPrPK->parDer1(evalqTRel(),getTime());
        PJT[0].set(i02,iuT,PJTT);
      }
      if(!constjT)
        PjhT = fPrPK->parDer2(evalqTRel(),getTime());
    }

    if(fAPK) {
      if(!constJR) {
        PJRR = fTR?fAPK->parDer1(evalqRRel(),getTime())*(*fTR)(evalqRRel()):fAPK->parDer1(evalqRRel(),getTime());
        PJR[0].set(i02,iuR,PJRR);
      }
      if(!constjR)
        PjhR = fAPK->parDer2(evalqRRel(),getTime());
    }
     updPJ = false;
  }

  void FlexibleBodyFFR::updateGyroscopicAccelerations() {
    VecV qdTRel = evaluTRel();
    VecV qdRRel = fTR ? (*fTR)(qRRel)*uRRel : uRRel;
    if(fPrPK) {
      if(not(constJT and constjT)) {
        PjbT = (fPrPK->parDer1DirDer1(qdTRel,qTRel,getTime())+fPrPK->parDer1ParDer2(qTRel,getTime()))*uTRel + fPrPK->parDer2DirDer1(qdTRel,qTRel,getTime()) + fPrPK->parDer2ParDer2(qTRel,getTime());
      }
    }
    if(fAPK) {
      if(not(constJR and constjR)) {
        if(fTR) {
          Mat3xV JRd = (fAPK->parDer1DirDer1(qdRRel,qRRel,getTime())+fAPK->parDer1ParDer2(qRRel,getTime()));
          MatV TRd = fTR->dirDer(qdRRel,qRRel);
          PjbR = JRd*qdRRel + fAPK->parDer1(qRRel,getTime())*TRd*uRRel + fAPK->parDer2DirDer1(qdRRel,qRRel,getTime()) + fAPK->parDer2ParDer2(qRRel,getTime());
        }
        else
          PjbR = (fAPK->parDer1DirDer1(qdRRel,qRRel,getTime())+fAPK->parDer1ParDer2(qRRel,getTime()))*uRRel + fAPK->parDer2DirDer1(qdRRel,qRRel,getTime()) + fAPK->parDer2ParDer2(qRRel,getTime());
      }
    }
    updPjb = false;
  }

  void FlexibleBodyFFR::updateKJ0() {
    KJ[0].set(Index(0,2),Index(0,KJ[0].cols()-1),K->evalOrientation().T()*K->evalJacobianOfTranslation());
    KJ[0].set(Index(3,5),Index(0,KJ[0].cols()-1),K->getOrientation().T()*K->getJacobianOfRotation());
    Ki.set(Index(0,2),K->getOrientation().T()*K->evalGyroscopicAccelerationOfTranslation());
    Ki.set(Index(3,5),K->getOrientation().T()*K->getGyroscopicAccelerationOfRotation());
    updKJ[0] = false;
  }

  void FlexibleBodyFFR::updateKJ1() {
    KJ[1].set(Index(0,2),Index(0,KJ[1].cols()-1),K->evalOrientation().T()*K->evalJacobianOfTranslation(1));
    KJ[1].set(Index(3,5),Index(0,KJ[1].cols()-1),K->getOrientation().T()*K->getJacobianOfRotation(1));
    KJ[1].set(Index(6,KJ[1].rows()-1),Index(0,KJ[1].cols()-1),K->getJacobianOfDeformation(1));
    updKJ[1] = false;
  }

  void FlexibleBodyFFR::updatePositions(Frame *frame) {
    frame->setPosition(R->getPosition() + evalGlobalRelativePosition());
    frame->setOrientation(R->getOrientation()*APK); // APK already update to date
  }

 void FlexibleBodyFFR::updateVelocities(Frame *frame) {
    frame->setAngularVelocity(R->evalAngularVelocity() + evalGlobalRelativeAngularVelocity());
    frame->setVelocity(R->getVelocity() + WvPKrel + crossProduct(R->getAngularVelocity(),evalGlobalRelativePosition())); // WvPKrel already update to date
  }

  void FlexibleBodyFFR::updateAccelerations(Frame *frame) {
    frame->setAcceleration(K->evalJacobianOfTranslation()*evaludall() + K->evalGyroscopicAccelerationOfTranslation());
    frame->setAngularAcceleration(K->evalJacobianOfRotation()*udall + K->evalGyroscopicAccelerationOfRotation());
  }

  void FlexibleBodyFFR::updateJacobians0(Frame *frame) {
    frame->getJacobianOfTranslation(0,false).init(0);
    frame->getJacobianOfRotation(0,false).init(0);
    frame->getJacobianOfTranslation(0,false).set(i02,Index(0,R->gethSize()-1), R->evalJacobianOfTranslation() - tilde(evalGlobalRelativePosition())*R->evalJacobianOfRotation());
    frame->getJacobianOfRotation(0,false).set(i02,Index(0,R->gethSize()-1), R->evalJacobianOfRotation());
    frame->getJacobianOfTranslation(0,false).add(i02,Index(gethSize(0)-getuSize(0),gethSize(0)-1), R->evalOrientation()*evalPJT());
    frame->getJacobianOfRotation(0,false).add(i02,Index(gethSize(0)-getuSize(0),gethSize(0)-1), frameForJacobianOfRotation->evalOrientation()*PJR[0]);
  }

  void FlexibleBodyFFR::updateGyroscopicAccelerations(Frame *frame) {
    frame->setGyroscopicAccelerationOfTranslation(R->evalGyroscopicAccelerationOfTranslation() + crossProduct(R->evalGyroscopicAccelerationOfRotation(),evalGlobalRelativePosition()) + R->evalOrientation()*evalPjbT() + crossProduct(R->evalAngularVelocity(), 2.*evalGlobalRelativeVelocity()+crossProduct(R->evalAngularVelocity(),evalGlobalRelativePosition())));
    frame->setGyroscopicAccelerationOfRotation(R->evalGyroscopicAccelerationOfRotation() + frameForJacobianOfRotation->evalOrientation()*PjbR + crossProduct(R->evalAngularVelocity(), evalGlobalRelativeAngularVelocity())); // PjbR already up to date
  }

  void FlexibleBodyFFR::resetUpToDate() {
    Body::resetUpToDate();
    updPjb = true;
    updGC = true;
    updT = true;
    updMb = true;
    updKJ[0] = true;
    updKJ[1] = true;
  }

//  void FlexibleBodyFFR::updateJacobiansForRemainingFramesAndContours1() {
//  }

  void FlexibleBodyFFR::updateqRef(const Vec& ref) {
    Body::updateqRef(ref);
    qRel>>q;
    for(unsigned int i=1; i<frame.size(); i++)
      static_cast<FixedNodalFrame*>(frame[i])->updateqRef(q(nq-ne,nq-1));
  }

  void FlexibleBodyFFR::updateuRef(const Vec& ref) {
    Body::updateuRef(ref);
    uRel>>u;
    for(unsigned int i=1; i<frame.size(); i++)
      static_cast<FixedNodalFrame*>(frame[i])->updateqdRef(u(nu[0]-ne,nu[0]-1));
  }

  void FlexibleBodyFFR::updateudRef(const fmatvec::Vec& ref) {
    Body::updateudRef(ref);
    for(unsigned int i=1; i<frame.size(); i++)
      static_cast<FixedNodalFrame*>(frame[i])->updateqddRef(ud(nu[0]-ne,nu[0]-1));
  }

  void FlexibleBodyFFR::addFrame(FixedNodalFrame *frame_) {
    Body::addFrame(frame_);
  }

#ifdef HAVE_OPENMBVCPPINTERFACE
  void FlexibleBodyFFR::setOpenMBVRigidBody(const std::shared_ptr<OpenMBV::RigidBody> &body) {
    openMBVBody=body;
  }
#endif

  void FlexibleBodyFFR::updateMConst() {
    M += Mbuf;
  }

  void FlexibleBodyFFR::updateMNotConst() {
    M += JTMJ(evalMb(),evalKJ());
  }

  void FlexibleBodyFFR::initializeUsingXML(DOMElement *element) {
    DOMElement *e;
    Body::initializeUsingXML(element);

    // frames
    e=E(element)->getFirstElementChildNamed(MBSIM%"frames")->getFirstElementChild();
    while(e) {
      FixedNodalFrame *f=new FixedNodalFrame(E(e)->getAttribute("name"));
      addFrame(f);
      f->initializeUsingXML(e);
      e=e->getNextElementSibling();
    }

    // contours
    e=E(element)->getFirstElementChildNamed(MBSIM%"contours")->getFirstElementChild();
    while(e) {
      Contour *c=ObjectFactory::createAndInit<Contour>(e);
      addContour(c);
      e=e->getNextElementSibling();
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"mass");
    setMass(getDouble(e));
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(VecV,double)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(VecV,double)> >(e->getFirstElementChild());
      setGeneralTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(double)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(double)> >(e->getFirstElementChild());
      setTimeDependentTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentTranslation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<Vec3(VecV)> *trans=ObjectFactory::createAndInit<MBSim::Function<Vec3(VecV)> >(e->getFirstElementChild());
      setStateDependentTranslation(trans);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"generalRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(VecV,double)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(VecV,double)> >(e->getFirstElementChild());
      setGeneralRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(double)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(double)> >(e->getFirstElementChild());
      setTimeDependentRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentRotation");
    if(e && e->getFirstElementChild()) {
      MBSim::Function<RotMat3(VecV)> *rot=ObjectFactory::createAndInit<MBSim::Function<RotMat3(VecV)> >(e->getFirstElementChild());
      setStateDependentRotation(rot);
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"translationDependentRotation");
    if(e) translationDependentRotation = getBool(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"coordinateTransformationForRotation");
    if(e) coordinateTransformation = getBool(e);

#ifdef HAVE_OPENMBVCPPINTERFACE
    e=E(element)->getFirstElementChildNamed(MBSIM%"openMBVRigidBody");
    if(e) {
      std::shared_ptr<OpenMBV::RigidBody> rb=OpenMBV::ObjectFactory::create<OpenMBV::RigidBody>(e->getFirstElementChild());
      setOpenMBVRigidBody(rb);
      rb->initializeUsingXML(e->getFirstElementChild());
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"openMBVFrameOfReference");
    if(e) setOpenMBVFrameOfReference(getByPath<Frame>(E(e)->getAttribute("ref"))); // must be on of "Frame[X]" which allready exists

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVFrameC");
    if(e) {
      if(!openMBVBody) setOpenMBVRigidBody(OpenMBV::ObjectFactory::create<OpenMBV::InvisibleBody>());
      OpenMBVFrame ombv;
      K->setOpenMBVFrame(ombv.createOpenMBV(e));
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVWeight");
    if(e) {
      if(!openMBVBody) setOpenMBVRigidBody(OpenMBV::ObjectFactory::create<OpenMBV::InvisibleBody>());
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FWeight=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointForce");
    if (e) {
      if(!openMBVBody) setOpenMBVRigidBody(OpenMBV::ObjectFactory::create<OpenMBV::InvisibleBody>());
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVJointMoment");
    if (e) {
      if(!openMBVBody) setOpenMBVRigidBody(OpenMBV::ObjectFactory::create<OpenMBV::InvisibleBody>());
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
#endif
  }

  DOMElement* FlexibleBodyFFR::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Body::writeXMLFile(parent);
    return ele0;
  }

}
