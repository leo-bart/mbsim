/* Copyright (C) 2004-2010 MBSim Development Team
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
#include "mbsim/constraint.h"
#include "mbsim/rigid_body.h"
#include "mbsim/frame.h"
#include "mbsim/utils/nonlinear_algebra.h"
#include "mbsim/utils/utils.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/joint.h"
#include "mbsim/gear.h"
#include "mbsim/kinematic_excitation.h"
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/constitutive_laws.h"
#include "mbsim/objectfactory.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/frame.h>
#endif

#include <hdf5serie/simpledataset.h>

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  Constraint::Constraint(const std::string &name) : Element(name) {
    setPlotFeature(state, enabled);
  }

  void Constraint::updatexRef(const Vec &xParent) {
    x >> xParent(xInd,xInd+xSize-1);
  } 

  void Constraint::updatexdRef(const Vec &xdParent) {
    xd >> xdParent(xInd,xInd+xSize-1);
  } 

  void Constraint::init(InitStage stage) {
    if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
        if(getPlotFeature(state)==enabled)
          for(int i=0; i<xSize; ++i)
            plotColumns.push_back("x("+numtostr(i)+")");
        if(getPlotFeature(stateDerivative)==enabled)
          for(int i=0; i<xSize; ++i)
            plotColumns.push_back("xd("+numtostr(i)+")");

        Element::init(stage);
      }
    }
    else
      Element::init(stage);
  }

  void Constraint::initz() {
    x = (x0.size()==0)? Vec(xSize, INIT, 0) : x0;
  }

  void Constraint::writez(H5::GroupBase *group) {
    group->createChildObject<H5::SimpleDataset<vector<double> > >("x0")(x.size())->write(x);
  }

  void Constraint::readz0(H5::GroupBase *group) {
    x0.resize() = group->openChildObject<H5::SimpleDataset<vector<double> > >("x0")->read();
  }

  void Constraint::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      if(getPlotFeature(state)==enabled)
        for(int i=0; i<xSize; ++i)
          plotVector.push_back(x(i));
      if(getPlotFeature(stateDerivative)==enabled)
        for(int i=0; i<xSize; ++i)
          plotVector.push_back(xd(i)/dt);

      Element::plot(t,dt);
    }
  }

  void Constraint::closePlot() {
    if(getPlotFeature(plotRecursive)==enabled) {
      Element::closePlot();
    }
  }

  JointConstraint::Residuum::Residuum(vector<RigidBody*> body1_, vector<RigidBody*> body2_, const Mat3xV &dT_, const Mat3xV &dR_,Frame *frame1_, Frame *frame2_,double t_,vector<Frame*> i1_, vector<Frame*> i2_) : body1(body1_),body2(body2_),dT(dT_),dR(dR_),frame1(frame1_), frame2(frame2_), t(t_), i1(i1_), i2(i2_) {}
  Vec JointConstraint::Residuum::operator()(const Vec &x) {
    Vec res(x.size(),NONINIT); 
    int nq = 0;
    for(unsigned int i=0; i<body1.size(); i++) {
      int dq = body1[i]->getqRel().size();
      body1[i]->resetUpToDate();
      body1[i]->setqRel(x(nq,nq+dq-1));
      nq += dq;
    }
    for(unsigned int i=0; i<body2.size(); i++) {
      int dq = body2[i]->getqRel().size();
      body2[i]->resetUpToDate();
      body2[i]->setqRel(x(nq,nq+dq-1));
      nq += dq;
    }

//    for(unsigned int i=0; i<body1.size(); i++) {
//      body1[i]->updatePositionAndOrientationOfFrame(t,i1[i]);
//    }
//    for(unsigned int i=0; i<body2.size(); i++) {
//      body2[i]->updatePositionAndOrientationOfFrame(t,i2[i]);
//    }

    int nT = dT.cols();
    int nR = dR.cols();

    if(nT) 
      res(Range<Var,Var>(0,nT-1)) = dT.T()*(frame1->getPosition(t)-frame2->getPosition(t)); 

    if(nR) 
      res(Range<Var,Var>(nT,nT+nR-1)) = dR.T()*AIK2Cardan(frame1->getOrientation().T()*frame2->getOrientation()); 

    return res;
  } 

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(GearConstraint, MBSIM%"GearConstraint")

  GearConstraint::GearConstraint(const std::string &name) : Constraint(name), bd(NULL), saved_DependentBody("") {
  }

  void GearConstraint::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if (saved_DependentBody!="")
        setDependentBody(getByPath<RigidBody>(saved_DependentBody));
      if (saved_IndependentBody.size()>0) {
        for (unsigned int i=0; i<saved_IndependentBody.size(); i++)
          bi.push_back(getByPath<RigidBody>(saved_IndependentBody[i]));
      }
      Constraint::init(stage);
    }
    else if(stage==preInit) {
      Constraint::init(stage);
      bd->addDependency(this);
      for(unsigned int i=0; i<bi.size(); i++)
        addDependency(bi[i]);
    }
    else
      Constraint::init(stage);
  }

  void GearConstraint::addTransmission(const Transmission &transmission) {
    bi.push_back(transmission.body); 
    ratio.push_back(transmission.ratio);
  }

  void GearConstraint::updateGeneralizedCoordinates(double t) {
    bd->getqRel().init(0);
    bd->getuRel().init(0);
    for(unsigned int i=0; i<bi.size(); i++) {
      bd->getqRel() += bi[i]->getqRel(t)*ratio[i];
      bd->getuRel() += bi[i]->getuRel(t)*ratio[i];
    }
  }

  void GearConstraint::updateGeneralizedJacobians(double t, int j) {
    bd->getJRel().init(0); 
    for(unsigned int i=0; i<bi.size(); i++) {
      bd->getJRel()(Range<Var,Var>(0,bi[i]->getJRel().rows()-1),Range<Var,Var>(0,bi[i]->getJRel().cols()-1)) += bi[i]->getJRel(t)*ratio[i];
    }
  }

  void GearConstraint::initializeUsingXML(DOMElement* element) {
    Constraint::initializeUsingXML(element);
    DOMElement *e, *ee;
    e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBody");
    saved_DependentBody=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIM%"transmissions");
    ee=e->getFirstElementChild();
    while(ee && E(ee)->getTagName()==MBSIM%"Transmission") {
      saved_IndependentBody.push_back(E(E(ee)->getFirstElementChildNamed(MBSIM%"rigidBody"))->getAttribute("ref"));
      ratio.push_back(getDouble(E(ee)->getFirstElementChildNamed(MBSIM%"ratio")));
      ee=ee->getNextElementSibling();
    }

#ifdef HAVE_OPENMBVCPPINTERFACE
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
#endif
  }

  void GearConstraint::setUpInverseKinetics() {
    Gear *gear = new Gear(string("Gear")+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(gear);
    gear->setDependentBody(bd);
    for(unsigned int i=0; i<bi.size(); i++)
      gear->addTransmission(Transmission(bi[i],ratio[i]));
    if(FArrow)
      gear->setOpenMBVForce(FArrow);
    if(MArrow)
      gear->setOpenMBVMoment(MArrow);
  }
//
//  KinematicConstraint::KinematicConstraint(const std::string &name) : Constraint(name), bd(0), saved_DependentBody("") {
//  }
//
//  void KinematicConstraint::init(InitStage stage) {
//    if(stage==resolveXMLPath) {
//      if (saved_DependentBody!="")
//        setDependentBody(getByPath<RigidBody>(saved_DependentBody));
//      Constraint::init(stage);
//    }
//    else if(stage==preInit) {
//      Constraint::init(stage);
//      bd->addDependency(this);
//    }
//    else
//      Constraint::init(stage);
//  }
//
//  void KinematicConstraint::initializeUsingXML(DOMElement* element) {
//    Constraint::initializeUsingXML(element);
//    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBody");
//    saved_DependentBody=E(e)->getAttribute("ref");
//
//#ifdef HAVE_OPENMBVCPPINTERFACE
//    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
//    if (e) {
//      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
//      FArrow=ombv.createOpenMBV(e);
//    }
//
//    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
//    if (e) {
//      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
//      MArrow=ombv.createOpenMBV(e);
//    }
//#endif
//  }
//
//  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(GeneralizedPositionConstraint, MBSIM%"GeneralizedPositionConstraint")
//
//  void GeneralizedPositionConstraint::init(InitStage stage) {
//    KinematicConstraint::init(stage);
//    f->init(stage);
//  }
//
//  void GeneralizedPositionConstraint::updateStateDependentVariables(double t) {
//    bd->getqRel() = (*f)(t);
//    bd->getuRel() = f->parDer(t);
//  }
//
//  void GeneralizedPositionConstraint::updateJacobians(double t, int jj) {
//    bd->getjRel() = f->parDerParDer(t);
//  }
//
//  void GeneralizedPositionConstraint::initializeUsingXML(DOMElement* element) {
//    KinematicConstraint::initializeUsingXML(element);
//    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"constraintFunction");
//    if(e) {
//      Function<VecV(double)> *f=ObjectFactory::createAndInit<Function<VecV(double)> >(e->getFirstElementChild());
//      setConstraintFunction(f);
//    }
//  }
//
//  void GeneralizedPositionConstraint::setUpInverseKinetics() {
//    GeneralizedPositionExcitation *ke = new GeneralizedPositionExcitation(string("GeneralizedPositionExcitation")+name);
//    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(ke);
//    ke->setDependentBody(bd);
//    ke->setExcitationFunction(f);
//    if(FArrow)
//      ke->setOpenMBVForce(FArrow);
//    if(MArrow)
//      ke->setOpenMBVMoment(MArrow);
//  }
//
//  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(GeneralizedVelocityConstraint, MBSIM%"GeneralizedVelocityConstraint")
//
//  void GeneralizedVelocityConstraint::init(InitStage stage) {
//    KinematicConstraint::init(stage);
//    f->init(stage);
//  }
//
//  void GeneralizedVelocityConstraint::calcxSize() {
//    xSize = bd->getqRelSize();
//  }
//
//  void GeneralizedVelocityConstraint::updatexd(double t) {
//    xd = bd->transformCoordinates()?bd->getTRel()*bd->getuRel():bd->getuRel();
//  }
//
//  void GeneralizedVelocityConstraint::updateStateDependentVariables(double t) {
//    bd->getqRel() = x;
//    bd->getuRel() = (*f)(x,t);
//  }
//
//  void GeneralizedVelocityConstraint::updateJacobians(double t, int jj) {
//    MatV J = f->parDer1(x,t);
//    if(J.cols())
//      bd->getjRel() = J*xd + f->parDer2(x,t);
//    else
//      bd->getjRel() = f->parDer2(x,t);
//  }
//
//  void GeneralizedVelocityConstraint::initializeUsingXML(DOMElement* element) {
//    KinematicConstraint::initializeUsingXML(element);
//    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"initialState");
//    if (e)
//      x0 = getVec(e);
//    e=E(element)->getFirstElementChildNamed(MBSIM%"generalConstraintFunction");
//    if(e) {
//      Function<VecV(VecV,double)> *f=ObjectFactory::createAndInit<Function<VecV(VecV,double)> >(e->getFirstElementChild());
//      setGeneralConstraintFunction(f);
//    }
//    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentConstraintFunction");
//    if(e) {
//      Function<VecV(double)> *f=ObjectFactory::createAndInit<Function<VecV(double)> >(e->getFirstElementChild());
//      setTimeDependentConstraintFunction(f);
//    }
//    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentConstraintFunction");
//    if(e) {
//      Function<VecV(VecV)> *f=ObjectFactory::createAndInit<Function<VecV(VecV)> >(e->getFirstElementChild());
//      setStateDependentConstraintFunction(f);
//    }
//  }
//
//  void GeneralizedVelocityConstraint::setUpInverseKinetics() {
//    GeneralizedVelocityExcitation *ke = new GeneralizedVelocityExcitation(string("GeneralizedVelocityExcitation")+name);
//    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(ke);
//    ke->setDependentBody(bd);
//    ke->setExcitationFunction(f);
//    if(FArrow)
//      ke->setOpenMBVForce(FArrow);
//    if(MArrow)
//      ke->setOpenMBVMoment(MArrow);
//  }
//
//  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(GeneralizedAccelerationConstraint, MBSIM%"GeneralizedAccelerationConstraint")
//
//  void GeneralizedAccelerationConstraint::init(InitStage stage) {
//    KinematicConstraint::init(stage);
//    f->init(stage);
//  }
//  
//  void GeneralizedAccelerationConstraint::calcxSize() {
//    xSize = bd->getqRelSize()+bd->getuRelSize();
//  }
//
//  void GeneralizedAccelerationConstraint::updatexd(double t) {
//    xd(0,bd->getqRelSize()-1) = bd->transformCoordinates()?bd->getTRel()*bd->getuRel():bd->getuRel();
//    xd(bd->getqRelSize(),bd->getqRelSize()+bd->getuRelSize()-1) = bd->getjRel();
//  }
//
//  void GeneralizedAccelerationConstraint::updateStateDependentVariables(double t) {
//    bd->getqRel() = x(0,bd->getqRelSize()-1);
//    bd->getuRel() = x(bd->getqRelSize(),bd->getqRelSize()+bd->getuRelSize()-1);
//  }
//
//  void GeneralizedAccelerationConstraint::updateJacobians(double t, int jj) {
//    bd->getjRel() = (*f)(x,t);
//  }
//
//  void GeneralizedAccelerationConstraint::initializeUsingXML(DOMElement* element) {
//    KinematicConstraint::initializeUsingXML(element);
//    DOMElement *e=E(element)->getFirstElementChildNamed(MBSIM%"initialState");
//    if(e)
//      x0 = getVec(e);
//    e=E(element)->getFirstElementChildNamed(MBSIM%"generalConstraintFunction");
//    if(e) {
//      Function<VecV(VecV,double)> *f=ObjectFactory::createAndInit<Function<VecV(VecV,double)> >(e->getFirstElementChild());
//      setGeneralConstraintFunction(f);
//    }
//    e=E(element)->getFirstElementChildNamed(MBSIM%"timeDependentConstraintFunction");
//    if(e) {
//      Function<VecV(double)> *f=ObjectFactory::createAndInit<Function<VecV(double)> >(e->getFirstElementChild());
//      setTimeDependentConstraintFunction(f);
//    }
//    e=E(element)->getFirstElementChildNamed(MBSIM%"stateDependentConstraintFunction");
//    if(e) {
//      Function<VecV(VecV)> *f=ObjectFactory::createAndInit<Function<VecV(VecV)> >(e->getFirstElementChild());
//      setStateDependentConstraintFunction(f);
//    }
//  }
//
//  void GeneralizedAccelerationConstraint::setUpInverseKinetics() {
//    GeneralizedAccelerationExcitation *ke = new GeneralizedAccelerationExcitation(string("GeneralizedAccelerationExcitation")+name);
//    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(ke);
//    ke->setDependentBody(bd);
//    ke->setExcitationFunction(f);
//    if(FArrow)
//      ke->setOpenMBVForce(FArrow);
//    if(MArrow)
//      ke->setOpenMBVMoment(MArrow);
//  }
//
  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(JointConstraint, MBSIM%"JointConstraint")

  JointConstraint::JointConstraint(const string &name) : Constraint(name), bi(NULL), bi2(NULL), frame1(0), frame2(0), refFrame(NULL), refFrameID(0), nq(0), nu(0), nh(0), saved_ref1(""), saved_ref2("") {
  }

  void JointConstraint::connect(Frame* frame1_, Frame* frame2_) {
    frame1 = frame1_;
    frame2 = frame2_;
  }

  void JointConstraint::setDependentBodiesFirstSide(vector<RigidBody*> bd) {    
    bd1 = bd;
  }

  void JointConstraint::setDependentBodiesSecondSide(vector<RigidBody*> bd) {
    bd2 = bd;
  }

  void JointConstraint::setIndependentBody(RigidBody *bi_) {
    bi = bi_;
  }

  void JointConstraint::setSecondIndependentBody(RigidBody *bi2_) {
    bi2 = bi2_;
  }

  void JointConstraint::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_ref1!="" && saved_ref2!="")
        connect(getByPath<Frame>(saved_ref1), getByPath<Frame>(saved_ref2));
      vector<RigidBody*> rigidBodies;
      if (saved_RigidBodyFirstSide.size()>0) {
        for (unsigned int i=0; i<saved_RigidBodyFirstSide.size(); i++)
          rigidBodies.push_back(getByPath<RigidBody>(saved_RigidBodyFirstSide[i]));
        setDependentBodiesFirstSide(rigidBodies);
      }
      rigidBodies.clear();
      if (saved_RigidBodySecondSide.size()>0) {
        for (unsigned int i=0; i<saved_RigidBodySecondSide.size(); i++)
          rigidBodies.push_back(getByPath<RigidBody>(saved_RigidBodySecondSide[i]));
        setDependentBodiesSecondSide(rigidBodies);
      }
      rigidBodies.clear();
      if (saved_IndependentBody!="")
        setIndependentBody(getByPath<RigidBody>(saved_IndependentBody));
      if (saved_IndependentBody2!="")
        setIndependentBody(getByPath<RigidBody>(saved_IndependentBody2));
      if(bd1.size()) {
        for(unsigned int i=0; i<bd1.size()-1; i++) 
          if1.push_back(bd1[i+1]->getFrameOfReference());
        if1.push_back(frame1);
      }
      if(bd2.size()) {
        for(unsigned int i=0; i<bd2.size()-1; i++) 
          if2.push_back(bd2[i+1]->getFrameOfReference());
        if2.push_back(frame2);
      }
      Constraint::init(stage);
    }
    else if(stage==preInit) {
      Constraint::init(stage);
      for(unsigned int i=0; i<bd1.size(); i++) 
        bd1[i]->addDependency(this);
      for(unsigned int i=0; i<bd2.size(); i++)
        bd2[i]->addDependency(this);
      if(bi)
        addDependency(bi);
      if(bi2)
        addDependency(bi2);
    } 
    else if(stage==unknownStage) {
      refFrame=refFrameID?frame2:frame1;
    } else
      Constraint::init(stage);
  }

  void JointConstraint::initz() {
    nq = 0;
    nu = 0;
    nh = 0;
    for(unsigned int i=0; i<bd1.size(); i++) {
      int dq = bd1[i]->getqRel().size();
      int du = bd1[i]->getuRel().size();
      int dh = bd1[i]->gethSize(0);
      Iq1.push_back(Index(nq,nq+dq-1));
      Iu1.push_back(Index(nu,nu+du-1));
      Ih1.push_back(Index(0,dh-1));
      nq += dq;
      nu += du;
      nh = max(nh,dh);
    }
    for(unsigned int i=0; i<bd2.size(); i++) {
      int dq = bd2[i]->getqRel().size();
      int du = bd2[i]->getuRel().size();
      int dh = bd2[i]->gethSize(0);
      Iq2.push_back(Index(nq,nq+dq-1));
      Iu2.push_back(Index(nu,nu+du-1));
      Ih2.push_back(Index(0,dh-1));
      nq += dq;
      nu += du;
      nh = max(nh,dh);
    }

    q.resize(nq);
    u.resize(nu);
    J.resize(nu,nh);
    j.resize(nu);
    JT.resize(3,nu);
    JR.resize(3,nu);
    for(unsigned int i=0; i<bd1.size(); i++) {
      bd1[i]->getqRel() >> q(Iq1[i]);
      bd1[i]->getuRel() >> u(Iu1[i]);
      bd1[i]->getJRel() >> J(Iu1[i],Ih1[i]);
      bd1[i]->getjRel() >> j(Iu1[i]); 
    }
    for(unsigned int i=0; i<bd2.size(); i++) {
      bd2[i]->getqRel() >> q(Iq2[i]);
      bd2[i]->getuRel() >> u(Iu2[i]);
      bd2[i]->getJRel() >> J(Iu2[i],Ih2[i]);
      bd2[i]->getjRel() >> j(Iu2[i]); 
    }   
    if(q0.size())
      q = q0;
  }

  void JointConstraint::updateGeneralizedCoordinates(double t) {
    dT = forceDir;
    dR = momentDir;

    Residuum f(bd1,bd2,dT,dR,frame1,frame2,t,if1,if2);
    MultiDimNewtonMethod newton(&f);
    q = newton.solve(q);
    if(newton.getInfo()!=0)
      msg(Warn) << endl << "Error in JointConstraint: update of state dependent variables failed!" << endl;

    for(unsigned int i=0; i<bd1.size(); i++) {
      bd1[i]->updateRelativeJacobians(t,if1[i]);
      for(unsigned int j=i+1; j<bd1.size(); j++) 
        bd1[j]->updateRelativeJacobians(t,if1[j],bd1[i]->getWJTrel(),bd1[i]->getWJRrel());
    }

    for(unsigned int i=0; i<bd2.size(); i++) {
      bd2[i]->updateRelativeJacobians(t,if2[i]);
      for(unsigned int j=i+1; j<bd2.size(); j++) 
        bd2[j]->updateRelativeJacobians(t,if2[j],bd2[i]->getWJTrel(),bd2[i]->getWJRrel());
    }

    for(unsigned int i=0; i<bd1.size(); i++) {
      JT(Index(0,2),Iu1[i]) = bd1[i]->getWJTrel();
      JR(Index(0,2),Iu1[i]) = bd1[i]->getWJRrel();
      //bd1[i]->resetUpToDate();
      bd1[i]->setqRel(bd1[i]->getqRel());
      bd1[i]->setuRel(Vec(1));
    }
    for(unsigned int i=0; i<bd2.size(); i++) {
      JT(Index(0,2),Iu2[i]) = -bd2[i]->getWJTrel();
      JR(Index(0,2),Iu2[i]) = -bd2[i]->getWJRrel();
      //bd2[i]->resetUpToDate();
      bd2[i]->setqRel(bd2[i]->getqRel());
      bd2[i]->setuRel(Vec(1));
    }
    SqrMat A(nu);
    A(Index(0,dT.cols()-1),Index(0,nu-1)) = dT.T()*JT;
    A(Index(dT.cols(),dT.cols()+dR.cols()-1),Index(0,nu-1)) = dR.T()*JR;
    Vec b(nu);

    b(0,dT.cols()-1) = -(dT.T()*(frame1->getVelocity(t)-frame2->getVelocity(t)));
    b(dT.cols(),dT.cols()+dR.cols()-1) = -(dR.T()*(frame1->getAngularVelocity(t)-frame2->getAngularVelocity(t)));
    u = slvLU(A,b); 
    for(unsigned int i=0; i<bd1.size(); i++) {
      bd1[i]->resetUpToDate();
      bd1[i]->setuRel(bd1[i]->getuRel());
    }
    for(unsigned int i=0; i<bd2.size(); i++) {
      bd2[i]->resetUpToDate();
      bd2[i]->setuRel(bd2[i]->getuRel());
    }
    updGC = false;
  }

  void JointConstraint::updateGeneralizedJacobians(double t, int jj) {
    if(jj == 0) {

      for(unsigned int i=0; i<bd1.size(); i++) {
        bd1[i]->setJRel(Mat(bd1[i]->getJRel().rows(),bd1[i]->getJRel().cols()));
        bd1[i]->setjRel(Vec(bd1[i]->getjRel().size()));
      }
      for(unsigned int i=0; i<bd2.size(); i++) {
        bd2[i]->setJRel(Mat(bd2[i]->getJRel().rows(),bd2[i]->getJRel().cols()));
        bd2[i]->setjRel(Vec(bd2[i]->getjRel().size()));
      }

      SqrMat A(nu);
      A(Index(0,dT.cols()-1),Index(0,nu-1)) = dT.T()*JT;
      A(Index(dT.cols(),dT.cols()+dR.cols()-1),Index(0,nu-1)) = dR.T()*JR;
      Mat B(nu,nh);
      Mat JT0(3,nh);
      Mat JR0(3,nh);
      if(frame1->getJacobianOfTranslation().cols()) {
        JT0(Index(0,2),Index(0,frame1->getJacobianOfTranslation().cols()-1))+=frame1->getJacobianOfTranslation(t);
        JR0(Index(0,2),Index(0,frame1->getJacobianOfRotation().cols()-1))+=frame1->getJacobianOfRotation(t);
      }
      if(frame2->getJacobianOfTranslation().cols()) {
        JT0(Index(0,2),Index(0,frame2->getJacobianOfTranslation().cols()-1))-=frame2->getJacobianOfTranslation(t);
        JR0(Index(0,2),Index(0,frame2->getJacobianOfRotation().cols()-1))-=frame2->getJacobianOfRotation(t);
      }
      B(Index(0,dT.cols()-1),Index(0,nh-1)) = -(dT.T()*JT0);
      B(Index(dT.cols(),dT.cols()+dR.cols()-1),Index(0,nh-1)) = -(dR.T()*JR0);
      Vec b(nu);
      b(0,dT.cols()-1) = -(dT.T()*(frame1->getGyroscopicAccelerationOfTranslation(t)-frame2->getGyroscopicAccelerationOfTranslation(t)));
      b(dT.cols(),dT.cols()+dR.cols()-1) = -(dR.T()*(frame1->getGyroscopicAccelerationOfRotation(t)-frame2->getGyroscopicAccelerationOfRotation(t)));

      J = slvLU(A,B); 
      j = slvLU(A,b); 
      for(unsigned int i=0; i<bd1.size(); i++) {
        bd1[i]->resetUpToDate();
        bd1[i]->setJRel(bd1[i]->getJRel());
      }
      for(unsigned int i=0; i<bd2.size(); i++) {
       bd2[i]->resetUpToDate();
       bd2[i]->setJRel(bd2[i]->getJRel());
      }
      updGJ = false;
    }
  }

  void JointConstraint::setUpInverseKinetics() {
    InverseKineticsJoint *joint = new InverseKineticsJoint(string("Joint_")+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(joint);
    if(forceDir.cols())
      joint->setForceDirection(forceDir);
    if(momentDir.cols())
      joint->setMomentDirection(momentDir);
    joint->connect(frame1,frame2);
    if(FArrow)
      joint->setOpenMBVForce(FArrow);
    if(MArrow)
      joint->setOpenMBVMoment(MArrow);
  }

  void JointConstraint::initializeUsingXML(DOMElement *element) {
    DOMElement *e, *ee;
    Constraint::initializeUsingXML(element);
    e=E(element)->getFirstElementChildNamed(MBSIM%"initialGuess");
    if (e) setInitialGuess(getVec(e));
    e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBodiesFirstSide");
    ee=e->getFirstElementChild();
    while(ee) {
      saved_RigidBodyFirstSide.push_back(E(ee)->getAttribute("ref"));
      ee=ee->getNextElementSibling();
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBodiesSecondSide");
    ee=e->getFirstElementChild();
    while(ee) {
      saved_RigidBodySecondSide.push_back(E(ee)->getAttribute("ref"));
      ee=ee->getNextElementSibling();
    }
    e=E(element)->getFirstElementChildNamed(MBSIM%"independentRigidBody");
    saved_IndependentBody=E(e)->getAttribute("ref");

    e=E(element)->getFirstElementChildNamed(MBSIM%"secondIndependentRigidBody");
    if(e) saved_IndependentBody2=E(e)->getAttribute("ref");

    e=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReferenceID");
    if(e) refFrameID=getInt(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"forceDirection");
    if(e) setForceDirection(getMat3xV(e,0));
    e=E(element)->getFirstElementChildNamed(MBSIM%"momentDirection");
    if(e) setMomentDirection(getMat3xV(e,3));

    e=E(element)->getFirstElementChildNamed(MBSIM%"connect");
    saved_ref1=E(e)->getAttribute("ref1");
    saved_ref2=E(e)->getAttribute("ref2");

#ifdef HAVE_OPENMBVCPPINTERFACE
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
#endif
  }

  DOMElement* JointConstraint::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Constraint::writeXMLFile(parent);
//    if(q0.size()) 
//      addElementText(ele0,MBSIM%"initialGeneralizedPosition",q0);
//    DOMElement *ele1 = new DOMElement( MBSIM%"dependentRigidBodiesFirstSide" );
//    for(unsigned int i=0; i<bd1.size(); i++) {
//      DOMElement *ele2 = new DOMElement( MBSIM%"dependentRigidBody" );
//      ele2->SetAttribute("ref", bd1[i]->getXMLPath(this,true)); // relative path
//      ele1->LinkEndChild(ele2);
//    }
//    ele0->LinkEndChild(ele1);
//    ele1 = new DOMElement( MBSIM%"dependentRigidBodiesSecondSide" );
//    for(unsigned int i=0; i<bd2.size(); i++) {
//      DOMElement *ele2 = new DOMElement( MBSIM%"dependentRigidBody" );
//      ele2->SetAttribute("ref", bd2[i]->getXMLPath(this,true)); // relative path
//      ele1->LinkEndChild(ele2);
//    }
//    ele0->LinkEndChild(ele1);
//
//    ele1 = new DOMElement( MBSIM%"independentRigidBody" );
//    ele1->SetAttribute("ref", bi->getXMLPath(this,true)); // relative path
//    ele0->LinkEndChild(ele1);
//
//    if(dT.cols())
//      addElementText(ele0, MBSIM%"forceDirection", dT);
//    if(dR.cols())
//      addElementText(ele0, MBSIM%"momentDirection", dR);
//
//    ele1 = new DOMElement(MBSIM%"connect");
//    ele1->SetAttribute("ref1", frame1->getXMLPath(this,true)); // relative path
//    ele1->SetAttribute("ref2", frame2->getXMLPath(this,true)); // relative path
//    ele0->LinkEndChild(ele1);
//
//    if(FArrow) {
//      ele1 = new DOMElement( MBSIM%"openMBVJointForceArrow" );
//      FArrow->writeXMLFile(ele1);
//      ele0->LinkEndChild(ele1);
//    }
//
//    if(MArrow) {
//      ele1 = new DOMElement( MBSIM%"openMBVJointMomentArrow" );
//      MArrow->writeXMLFile(ele1);
//      ele0->LinkEndChild(ele1);
//    }

    return ele0;
  }

  void JointConstraint::setForceDirection(const Mat3xV &fd) {

    forceDir = fd;

    for(int i=0; i<fd.cols(); i++)
      forceDir.set(i, forceDir.col(i)/nrm2(fd.col(i)));
  }

  void JointConstraint::setMomentDirection(const Mat3xV &md) {

    momentDir = md;

    for(int i=0; i<md.cols(); i++)
      momentDir.set(i, momentDir.col(i)/nrm2(md.col(i)));
  }


}
