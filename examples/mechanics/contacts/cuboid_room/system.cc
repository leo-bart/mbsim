#include "system.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsim/links/contact.h"
#include "mbsim/contours/room.h"
#include "mbsim/contours/cuboid.h"
#include "openmbvcppinterface/cube.h"
#include <openmbvcppinterface/sphere.h>
#include "mbsim/environment.h"


using namespace fmatvec;

System::System(const string &projectName) : DynamicSystemSolver(projectName) {
  RigidBody* roomBody = new RigidBody("RaumKoerper");
  addObject(roomBody);
  roomBody->setMass(1);
  roomBody->setInertiaTensor(SymMat3(EYE));
  roomBody->setFrameOfReference(getFrameI());
  Room* room = new Room("Raum");
  room->setXLength(.3); //X
  room->setYLength(.2); //Y
  room->setZLength(0.1); //Z
  room->setFrameOfReference(I);
  room->enableOpenMBV(_transparency=0.5);
  roomBody->addContour(room);


  RigidBody* body = new RigidBody("Wuerfel");
  addObject(body);

  body->setFrameOfReference(getFrame("I"));
  body->setFrameForKinematics(body->getFrame("C"));

  Mat J("[1,0,0;0,1,0;0,0,1]");
  body->setTranslation(new TranslationAlongAxesXYZ<VecV>);
  body->setRotation(new RotationAboutAxesXYZ<VecV>);
  double m = 0.1;
  double l,b,h;
  l = 0.01;
  b = 0.01;
  h = 0.01;
  Vec q0(6);
  body->setGeneralizedInitialPosition(q0);
  body->setGeneralizedInitialVelocity("[2;3;1;3;2;-1]");
  body->setMass(m);
  SymMat Theta(3);
  double A=m/12.*(b*b+h*h);
  double  B=m/12.*(h*h+l*l);
  double  C=m/12.*(l*l+b*b);
  Theta(0,0)=A;
  Theta(1,1)=B;
  Theta(2,2)=C;
  body->setInertiaTensor(Theta);

//  Point* point = new Point("Punkt");
//  body->addContour(point, Vec3(), SqrMat3(EYE));
//
//  std::shared_ptr<OpenMBV::Sphere> sphere = OpenMBV::ObjectFactory::create<OpenMBV::Sphere>();
//  body->setOpenMBVRigidBody(sphere);
//  sphere->setRadius(0.01);

  Cuboid *cuboid = new Cuboid("Wuerfel");
  cuboid->setXLength(l);
  cuboid->setYLength(h);
  cuboid->setZLength(b);
  cuboid->setFrameOfReference(body->getFrameC());
  body->addContour(cuboid);

  Contact *cnf = new Contact("Kontakt_Wuerfel");
  cnf->setNormalForceLaw(new UnilateralConstraint);
  cnf->setNormalImpactLaw(new UnilateralNewtonImpact(1));
//  cnf->setTangentialForceLaw(new SpatialCoulombFriction(0.3));
//  cnf->setTangentialImpactLaw(new SpatialCoulombImpact(0.3));
  //cnf->setPlotFeature(linkKinematics,disabled);
  //cnf->setNormalForceLaw(new LinearRegularizedUnilateralConstraint(1e4,100));
  //cnf->setTangentialForceLaw(new LinearRegularizedSpatialCoulombFriction(0.3));
  cnf->connect(roomBody->getContour("Raum"), body->getContour("Wuerfel"));
  addLink(cnf);

  std::shared_ptr<OpenMBV::Cube> obj = OpenMBV::ObjectFactory::create<OpenMBV::Cube>();
  body->setOpenMBVRigidBody(obj);
  obj->setLength(l);

  setPlotFeatureRecursive("generalizedPosition",enabled);
  setPlotFeatureRecursive("generalizedVelocity",enabled);
  setPlotFeatureRecursive("generalizedRelativePosition",enabled);
  setPlotFeatureRecursive("generalizedRelativeVelocity",enabled);
  setPlotFeatureRecursive("generalizedForce",enabled);
}

