#include "system.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/joint.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/environment.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/kinetics/kinetics.h"
#include "mbsim/observers/mechanical_link_observer.h"

#include <openmbvcppinterface/frustum.h>

using namespace MBSim;
using namespace fmatvec;
using namespace std;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {

  Vec grav(3);
  grav(1)=-9.81;
  MBSimEnvironment::getInstance()->setAccelerationOfGravity(grav);

  double m1 = 0.5;
  double m2 = 0.5;
  double l1 = 1;
  double l2 = 1;
  double a1 = 0.2; 
  double b1 = 0.45; 
  double a2 = 0.2; 
  double phi1 = -M_PI/2;
  double phi2 = -M_PI/4;

  RigidBody *body1 = new RigidBody("Stab1");
  addObject(body1);
  body1->setMass(m1);
  SymMat Theta(3);
  Theta(2,2) = 1./12.*m1*l1*l1;
  body1->setInertiaTensor(Theta);

  SqrMat E(3);
  E << DiagMat(3,INIT,1);
  Vec KrSP(3);
  KrSP(1) = a1;
  body1->addFrame(new FixedRelativeFrame("PunktO",KrSP,E));

  body1->setTranslation(new TranslationAlongAxesXY<VecV>);
  body1->setRotation(new RotationAboutZAxis<VecV>);

  body1->setFrameOfReference(getFrame("I"));
  body1->setFrameForKinematics(body1->getFrame("C"));
  Vec q0(3);
  q0(0) = a1;
  q0(2) = -phi1;
  body1->setGeneralizedInitialPosition(q0);

  KrSP(1) = -b1;
  body1->addFrame(new FixedRelativeFrame("PunktU",KrSP,E));

  RigidBody *body2 = new RigidBody("Stab2");
  addObject(body2);
  body2->setMass(m2);
  Theta(2,2) = 1./12.*m2*l2*l2;
  body2->setInertiaTensor(Theta);

  KrSP(1) = a2;
  body2->addFrame(new FixedRelativeFrame("Punkt",KrSP,E));
  body2->setTranslation(new TranslationAlongAxesXY<VecV>);
  body2->setRotation(new RotationAboutZAxis<VecV>);

  SqrMat A1(3);
  A1(0,0) = cos(phi1);
  A1(0,1) = sin(phi1);
  A1(1,0) = -sin(phi1);
  A1(1,1) = cos(phi1);
  A1(2,2) = 1;
  SqrMat A2(3);
  A2(0,0) = cos(phi2);
  A2(0,1) = sin(phi2);
  A2(1,0) = -sin(phi2);
  A2(1,1) = cos(phi2);
  A2(2,2) = 1;
  Vec rOP(3);
  rOP(1) = -a1-b1;
  q0 = A1*rOP + A2*(-KrSP);
  q0(2) = -phi2;
  body2->setGeneralizedInitialPosition(q0);

  addFrame(new FixedRelativeFrame("Os","[0;0;0.04]",E));
  body2->setFrameOfReference(getFrame("Os"));
  body2->setFrameForKinematics(body2->getFrame("C"));

  Joint *joint1 = new Joint("Gelenk1");
  addLink(joint1);
  joint1->setForceDirection(Mat("[1,0; 0,1; 0,0]"));
  joint1->connect(getFrame("I"),body1->getFrame("PunktO"));

  Joint *joint2 = new Joint("Gelenk2");
  addLink(joint2);
  joint2->setForceDirection(Mat("[1,0; 0,1; 0,0]"));
  joint2->connect(body1->getFrame("PunktU"),body2->getFrame("Punkt"));

  joint1->setForceLaw(new BilateralConstraint);
  joint2->setForceLaw(new BilateralConstraint);

  std::shared_ptr<OpenMBV::Frustum> cylinder=OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
  cylinder->setTopRadius(0.02);
  cylinder->setBaseRadius(0.02);
  cylinder->setHeight(l1);
  cylinder->setInitialTranslation(0,-0.5,0);
  cylinder->setInitialRotation(1.5708,0,0);
  body1->setOpenMBVRigidBody(cylinder);

  cylinder=OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
  cylinder->setTopRadius(0.02);
  cylinder->setBaseRadius(0.02);
  cylinder->setHeight(l2);
  cylinder->setInitialTranslation(0,-0.5,0);
  cylinder->setInitialRotation(1.5708,0,0);
  body2->setOpenMBVRigidBody(cylinder);

  MechanicalLinkObserver *observer = new MechanicalLinkObserver("Observer1");
  addObserver(observer);
  observer->setMechanicalLink(joint1);
  observer->enableOpenMBVForce(0.02);

  observer = new MechanicalLinkObserver("Observer2");
  addObserver(observer);
  observer->setMechanicalLink(joint2);
  observer->enableOpenMBVForce(0.02);

  body1->setPlotFeature("generalizedPosition",Element::enabled);
  body1->setPlotFeature("generalizedVelocity",Element::enabled);
  body2->setPlotFeature("generalizedPosition",Element::enabled);
  body2->setPlotFeature("generalizedVelocity",Element::enabled);
  joint1->setPlotFeature("generalizedRelativePosition",Element::enabled);
  joint1->setPlotFeature("generalizedRelativeVelocity",Element::enabled);
  joint1->setPlotFeature("generalizedForce",Element::enabled);
  joint2->setPlotFeature("generalizedRelativePosition",Element::enabled);
  joint2->setPlotFeature("generalizedRelativeVelocity",Element::enabled);
  joint2->setPlotFeature("generalizedForce",Element::enabled);
}

