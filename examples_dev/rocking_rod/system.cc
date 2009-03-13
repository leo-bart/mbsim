#include "system.h"
#include "rigid_body.h"
#include "objobject.h"
#include "contour.h"
#include "contact.h"
#include "load.h"
#include "cube.h"
#include "constitutive_laws.h"

using namespace AMVis;

extern bool rigidContacts;

System::System(const string &projectName) : MultiBodySystem(projectName) {
 // Gravitation
  Vec grav(3);
  grav(1)=-9.81;
  setAccelerationOfGravity(grav);
 // Parameters
  double l = 0.8;              		
  double h =  0.02;
  double d = 0.1;
  double m = 0.7;
  SymMat Theta(3);
  Theta(1,1) = m*l*l/12.;
  Theta(2,2) = Theta(1,1);
  double alpha = 3.0 * M_PI/180.; 
  double deltax = 0.2;           
  double mu  = 0.3;

  RigidBody* body = new RigidBody("Rod");
  addObject(body);

  body->setFrameOfReference(getFrame("I"));
  body->setFrameForKinematics(body->getFrame("C"));
  body->setMass(m);
  body->setInertiaTensor(Theta);
  body->setTranslation(new LinearTranslation("[1, 0; 0, 1; 0, 0]"));
  body->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));

 // Initial translation and rotation
  Vec q0(3);
  q0(1) = .5;
  q0(2) = alpha-M_PI/2;
  body->setq0(q0);

  // Contour definition
  Line *line = new Line("Line");
  Vec KrSC(3);
  KrSC(0) = 0.5*h;
  body->addContour(line,KrSC,SqrMat(3,EYE));

  // Obstacles
  Vec delta1(3); 
  delta1(0) = -deltax/2.;
  Point* point1 = new Point("Point1");
  addContour(point1,delta1,SqrMat(3,EYE));

  Vec delta2(3);
  delta2(0) = deltax/2.;
  Point* point2 = new Point("Point2");
  addContour(point2,delta2,SqrMat(3,EYE));

  Contact *cr1S = new Contact("Contact1"); 
  cr1S->connect(point1,body->getContour("Line"));
  addLink(cr1S);

  Contact *cr2S = new Contact("Contact2");
  cr2S->connect(point2,body->getContour("Line"));
  //cr2S->setPlotLevel(2);
  addLink(cr2S);

  if(rigidContacts) {
    cr1S->setContactForceLaw(new UnilateralConstraint);
    cr1S->setContactImpactLaw(new UnilateralNewtonImpact);
    cr1S->setFrictionForceLaw(new PlanarCoulombFriction(mu));
    cr1S->setFrictionImpactLaw(new PlanarCoulombImpact(mu));
    cr2S->setContactForceLaw(new UnilateralConstraint);
    cr2S->setContactImpactLaw(new UnilateralNewtonImpact);
    cr2S->setFrictionForceLaw(new PlanarCoulombFriction(mu));
    cr2S->setFrictionImpactLaw(new PlanarCoulombImpact(mu));
  }
  else {
    cr1S->setContactForceLaw(new LinearRegularizedUnilateralConstraint(1e5,1e4));
    cr1S->setFrictionForceLaw(new LinearRegularizedPlanarCoulombFriction(mu));
    cr2S->setContactForceLaw(new LinearRegularizedUnilateralConstraint(1e5,1e4));
    cr2S->setFrictionForceLaw(new LinearRegularizedPlanarCoulombFriction(mu));
  }

  // Visualisation with AMVis
  ObjObject *obj = new ObjObject(getName() + "." + body->getName(),1,false);
  obj->setObjFilename("objects/rod.obj");
  body->setAMVisBody(obj);
  obj->setInitialRotation(M_PI/2,M_PI/2,0);
  obj->setScaleFactor(0.1);
  obj->setCalculationOfNormals(3);
  obj->setVertexEPS(1e-5);
  obj-> setNormalEPS(1e-5);
  obj-> setAngleEPS(M_PI*2/9);
  // Cuboid *cubeoid = new Cuboid(body->getFullName(),1,false);
  // cubeoid->setSize(l,h,d);
  // cubeoid->setColor(0);
  // body->setAMVisBody(cubeoid);

}

