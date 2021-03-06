#include "system.h"

#include "mbsimHydraulics/rigid_line.h"
#include "mbsimHydraulics/hnode.h"
#include "mbsimHydraulics/hnode_mec.h"
#include "mbsimHydraulics/pressure_loss.h"

#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/spring_damper.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/utils/utils.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/kinetics/kinetics.h"
#include "mbsim/functions/constant_function.h"

#include "openmbvcppinterface/polygonpoint.h"
#include "openmbvcppinterface/frustum.h"
#include "openmbvcppinterface/compoundrigidbody.h"
#include "openmbvcppinterface/extrusion.h"
#include "openmbvcppinterface/coilspring.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBSimHydraulics;

string getBodyName(int i) {
  string name;
  switch (i) {
    case 0:
      name="L";
      break;
    case 1:
      name="LM";
      break;
    case 2:
      name="M";
      break;
    case 3:
      name="RM";
      break;
    case 4:
      name="R";
      break;
  }
  return name;
}

shared_ptr<vector<shared_ptr<OpenMBV::PolygonPoint> > > createPiece(double rI, double rA, double phi0, double dphi) {
  int nI = int((dphi*rI)/(5e-2*rI));
  int nA = int((dphi*rA)/(5e-2*rI));

  shared_ptr<vector<shared_ptr<OpenMBV::PolygonPoint> > > vpp = make_shared<vector<shared_ptr<OpenMBV::PolygonPoint> > >();
  for (int i=0; i<=nI; i++) {
    double phi = phi0 + double(i)/double(nI) * dphi;
    double x=rI*cos(phi);
    double y=rI*sin(phi);
    int b=((i==0)||(i==nI))?1:0;
    vpp->push_back(OpenMBV::PolygonPoint::create(x, y, b));
  }
  for (int i=nA; i>=0; i--) {
    double phi = phi0 + double(i)/double(nA) * dphi;
    double x=rA*cos(phi);
    double y=rA*sin(phi);
    int b=((i==0)||(i==nA))?1:0;
    vpp->push_back(OpenMBV::PolygonPoint::create(x, y, b));
  }
  vpp->push_back((*vpp)[0]);

  return vpp;
}

System::System(const string &name, bool unilateral) : Group(name) {

  double dI=0.01;
  double dA=0.05;
  double h=0.02;
  double pRB=2e5;
  double cF=1e4;
  
  double phiSolid=20.*M_PI/180.;
  double phiFree=(2.*M_PI-5.*phiSolid)/5.;
  double dphi=2.*M_PI/5.;
  
  double factor=(dA*dA-dI*dI)/8.*h;
  
  double V0=phiFree*factor;

  double rM=dI/2.+(dA-dI)/4.;
  double l0=rM*sqrt(2-2.*cos(phiFree));
  double area=(dA-dI)/2.*h;
  cout << "area=" << area << endl;

  RigidBody * traeger = new RigidBody("Traeger");
  addObject(traeger);
  double phi=0;
  for (int i=0; i<5; i++) {
    traeger->addFrame(new FixedRelativeFrame(getBodyName(i), Vec(3), BasicRotAIKz(phi)));
    traeger->getFrame(getBodyName(i))->enableOpenMBV(h/2.);
    phi+=dphi;
  }
  traeger->setFrameOfReference(getFrame("I"));
  traeger->setFrameForKinematics(traeger->getFrame("C"));
  traeger->setMass(2.);
  traeger->setInertiaTensor(0.001*SymMat(3, EYE));
//  traeger->setTranslation(new LinearTranslation(SqrMat(3, EYE)));
//  traeger->setRotation(new CardanAngles());
//  traeger->setTranslation(new LinearTranslation(Mat("[0;0;1]")));
//  traeger->setRotation(new RotationAboutFixedAxis(Vec("[0;0;1]")));
  std::shared_ptr<OpenMBV::CompoundRigidBody> traegerVisu = OpenMBV::ObjectFactory::create<OpenMBV::CompoundRigidBody>();
  
  std::shared_ptr<OpenMBV::Frustum> traegerVisuBoden = OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
  traegerVisuBoden->setBaseRadius(dA/2.);
  traegerVisuBoden->setTopRadius(dA/2.);
  traegerVisuBoden->setHeight(h/4.);
  traegerVisuBoden->setInitialRotation(0, 0, 0);
  traegerVisuBoden->setInitialTranslation(0, 0, 0);
  traegerVisuBoden->setDiffuseColor(0.5,0.5,0.5);
  traegerVisuBoden->setName("frustum1");
  traegerVisu->addRigidBody(traegerVisuBoden);
  
  std::shared_ptr<OpenMBV::Frustum> traegerVisuMitte = OpenMBV::ObjectFactory::create<OpenMBV::Frustum>();
  traegerVisuMitte->setBaseRadius(dI/2.);
  traegerVisuMitte->setTopRadius(dI/2.);
  traegerVisuMitte->setHeight(h);
  traegerVisuMitte->setInitialRotation(0, 0, 0);
  traegerVisuMitte->setInitialTranslation(0, 0, h);
  traegerVisuMitte->setDiffuseColor(0.5,0.5,0.5);
  traegerVisuMitte->setName("frustum2");
  traegerVisu->addRigidBody(traegerVisuMitte);
  traeger->setOpenMBVRigidBody(traegerVisu);

  Vec r(3, INIT, 0);
  r(0)=rM;
  r(2)=h/2.;
  for (int i=0; i<5; i++) {
    RigidBody * scheibe = new RigidBody("Scheibe_"+getBodyName(i));
    addObject(scheibe);
    scheibe->setMass(2.);
    scheibe->addFrame(new FixedRelativeFrame("L", BasicRotAIKz(phiSolid/2.)*r, BasicRotAIKz(phiSolid/2.)));
    scheibe->getFrame("L")->enableOpenMBV(h/2.);
    scheibe->addFrame(new FixedRelativeFrame("R", BasicRotAIKz(-phiSolid/2.)*r, BasicRotAIKz(-phiSolid/2.)));
    scheibe->getFrame("R")->enableOpenMBV(h/2.);
    scheibe->setFrameOfReference(traeger->getFrame(getBodyName(i)));
    scheibe->setFrameForKinematics(scheibe->getFrame("C"));
    scheibe->setInertiaTensor(0.001*SymMat(3, EYE));
    if (i>0)
      scheibe->setRotation(new RotationAboutFixedAxis<VecV>(Vec("[0; 0; 1]")));
    std::shared_ptr<OpenMBV::Extrusion> scheibeVisu = OpenMBV::ObjectFactory::create<OpenMBV::Extrusion>();
    scheibeVisu->setHeight(h);
    scheibeVisu->addContour(createPiece(dI/2., dA/2., 0, phiSolid));
    scheibeVisu->setInitialRotation(0, 0, -phiSolid/2.);
    scheibeVisu->setDiffuseColor(i/4.,i/4.,i/4.);
    scheibe->setOpenMBVRigidBody(scheibeVisu);

    if (i>0) {
      SpringDamper * sp = new SpringDamper("Spring_"+getBodyName(i-1)+"_"+getBodyName(i));
      addLink(sp);
      sp->setForceFunction(new LinearSpringDamperForce(cF,0.05*cF));
      sp->setUnloadedLength(l0);
      sp->connect(
          dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(i-1)))->getFrame("L"), 
          dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(i)))->getFrame("R"));
      sp->enableOpenMBV(_springRadius=.75*.1*h,_crossSectionRadius=.1*.25*h,_numberOfCoils=5);
    }
  }
  SpringDamper * sp = new SpringDamper("Spring_"+getBodyName(4)+"_"+getBodyName(0));
  addLink(sp);
  sp->setForceFunction(new LinearSpringDamperForce(cF,0.05*cF));
  sp->setUnloadedLength(l0);
  sp->connect(
      dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(4)))->getFrame("L"), 
      dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(0)))->getFrame("R"));
  sp->enableOpenMBV(_springRadius=.75*.1*h,_crossSectionRadius=.1*.25*h,_numberOfCoils=5);

  RigidLine * l04 = new RigidLine("l04");
  addObject(l04);
  l04->setDiameter(5e-3);
  l04->setLength(.7);
  ZetaLinePressureLoss * zeta = new ZetaLinePressureLoss();
  zeta->setZeta(14);
  l04->setLinePressureLoss(zeta);
  l04->setFrameOfReference(getFrame("I"));
  l04->setDirection("[0;0;0]");
  
  ConstrainedNode * n0 = new ConstrainedNode("n0");
  addLink(n0);
  n0->setpFunction(new ConstantFunction<double(double)>(.9e5));
  n0->addOutFlow(l04);

  EnvironmentNodeMec * n1Inf = new EnvironmentNodeMec("n1Inf");
  addLink(n1Inf);
  n1Inf->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(0)))->getFrame("R"), "[0;1;0]", area, traeger->getFrame("C"));
  n1Inf->enableOpenMBVArrows(.01);
  
  ConstrainedNodeMec * n1 = new ConstrainedNodeMec("n_"+getBodyName(0)+"_"+getBodyName(1));
  addLink(n1);
  n1->enableOpenMBVSphere(.005);
  n1->setInitialVolume(V0);
  n1->setpFunction(new ConstantFunction<double(double)>(pRB));
  n1->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(1)))->getFrame("R"), "[0;1;0]", area, traeger->getFrame("C"));
  n1->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(0)))->getFrame("L"), "[0;-1;0]", area, traeger->getFrame("C"));
  n1->enableOpenMBVArrows(.01);

  ElasticNodeMec * n2 = new ElasticNodeMec("n_"+getBodyName(1)+"_"+getBodyName(2));
  n2->setFracAir(0.08);
  n2->setp0(10e5);
  addLink(n2);
  n2->enableOpenMBVArrows(.01);
  n2->enableOpenMBVSphere(.005);
  n2->setInitialVolume(V0);
  n2->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(2)))->getFrame("R"), Vec("[0;1;0]"), area, traeger->getFrame("C")); 
  n2->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(1)))->getFrame("L"), Vec("[0;-1;0]"), area, traeger->getFrame("C"));

  ElasticNodeMec * n3 = new ElasticNodeMec("n_"+getBodyName(2)+"_"+getBodyName(3));
  n3->setFracAir(0.08);
  n3->setp0(1e5);
  addLink(n3);
  n3->enableOpenMBVArrows(.01);
  n3->enableOpenMBVSphere(.005);
  n3->setInitialVolume(V0);
  n3->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(3)))->getFrame("R"), Vec("[0;1;0]"), area, traeger->getFrame("C"));
  n3->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(2)))->getFrame("L"), Vec("[0;-1;0]"), area, traeger->getFrame("C"));

  RigidNodeMec * n4 = new RigidNodeMec("n_"+getBodyName(3)+"_"+getBodyName(4));
  addLink(n4);
  n4->setInitialVolume(V0);
  n4->enableOpenMBVArrows(.01);
  n4->enableOpenMBVSphere(.005);
  n4->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(4)))->getFrame("R"), Vec("[0;1;0]"), area, traeger->getFrame("C")); 
  n4->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(3)))->getFrame("L"), Vec("[0;-1;0]"), area, traeger->getFrame("C"));
  n4->addInFlow(l04);

  EnvironmentNodeMec * n4Inf = new EnvironmentNodeMec("n4Inf");
  addLink(n4Inf);
  n4Inf->addRotMecArea(dynamic_cast<RigidBody*>(getObject("Scheibe_"+getBodyName(4)))->getFrame("L"), Vec("[0;-1;0]"), area, traeger->getFrame("C"));
  n4Inf->enableOpenMBVArrows(.01);

  setPlotFeatureRecursive("generalizedPosition",enabled);
  setPlotFeatureRecursive("generalizedVelocity",enabled);
  setPlotFeatureRecursive("derivativeOfGeneralizedPosition",enabled);
  setPlotFeatureRecursive("generalizedAcceleration",enabled);
  setPlotFeatureRecursive("generalizedRelativePosition",enabled);
  setPlotFeatureRecursive("generalizedRelativeVelocity",enabled);
  setPlotFeatureRecursive("generalizedForce",enabled);
  setPlotFeatureRecursive("deflection",enabled);
}
