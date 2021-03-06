#include "system.h"

#include "mbsimHydraulics/rigid_line.h"
#include "mbsimHydraulics/hnode.h"
#include "mbsimHydraulics/hnode_mec.h"
#include "mbsim/links/contact.h"
#include "mbsim/constitutive_laws/constitutive_laws.h"
#include "mbsimHydraulics/checkvalve.h"
#include "mbsim/frames/fixed_relative_frame.h"
#include "mbsim/objects/rigid_body.h"
#include "mbsim/links/spring_damper.h"
#include "mbsimHydraulics/pressure_loss.h"
#include "mbsim/functions/kinematics/kinematics.h"
#include "mbsim/functions/kinetics/kinetics.h"
#include "mbsim/functions/tabular_function.h"
#include "mbsim/functions/constant_function.h"
#include "mbsimControl/object_sensors.h"

#include "mbsim/utils/rotarymatrices.h"

#include <openmbvcppinterface/arrow.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBSimHydraulics;

System::System(const string &name, bool bilateral, bool unilateral) : Group(name) {

  addFrame(new FixedRelativeFrame("ref", Vec("[.01; .02; .00]"), BasicRotAIKy(1)*BasicRotAIKz(-1.)));

  RigidBody * b = new RigidBody("Body");
  addObject(b);
  b->setMass(1);
  b->setInertiaTensor(SymMat(3, EYE));
  b->setFrameOfReference(getFrame("ref"));
  b->addFrame(new FixedRelativeFrame("ref", Vec("[.01; .02; .00]"), BasicRotAIKy(-2.)*BasicRotAIKz(-1.)));
  b->setFrameForKinematics(b->getFrame("C"));
  b->setTranslation(new TranslationAlongXAxis<VecV>);
  b->setGeneralizedInitialVelocity(.1);

  Checkvalve * lCV = new Checkvalve("lCV");
  addGroup(lCV);
  if (unilateral)
    lCV->setLineSetValued();
  lCV->setFrameOfReference(b->getFrame("ref"));
  lCV->setLineLength(.05);
  lCV->setLineDiameter(4e-3);
  GammaCheckvalveClosablePressureLoss * lCVPressureLoss = new GammaCheckvalveClosablePressureLoss();
  lCVPressureLoss->setGamma(M_PI/4.);
  lCVPressureLoss->setAlpha(.1);
  lCVPressureLoss->setBallRadius(6e-3);
  lCV->setLinePressureLoss(lCVPressureLoss);
  lCV->setLineMinimalXOpen(1e-4);
  lCV->setBallMass(0.1);
  lCV->setSpringForceFunction(new LinearSpringDamperForce(200, 5));
  lCV->setSpringUnloadedLength(5e-3);
  double c=1e5;
  double d=1e3;
  if (unilateral) {
    lCV->setSeatContactForceLaw(new UnilateralConstraint);
    lCV->setSeatContactImpactLaw(new UnilateralNewtonImpact);
  }
  else
    lCV->setSeatContactForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(c, d)));
  lCV->setMaximalOpening(.003);
  if (unilateral) {
    lCV->setMaximalContactForceLaw(new UnilateralConstraint);
    lCV->setMaximalContactImpactLaw(new UnilateralNewtonImpact);
  }
  else
    lCV->setMaximalContactForceLaw(new RegularizedUnilateralConstraint(new LinearRegularizedUnilateralConstraint(c, d)));

  ConstrainedNodeMec * n1 = new ConstrainedNodeMec("n1");
  n1->setpFunction(new TabularFunction<double(double)>(Vec("[0; .9; 1.1; 2.9; 3.1; 5]")*.1, "[4e5; 4e5; 2e5; 2e5; 4e5; 4e5]"));
  addLink(n1);
  n1->addOutFlow(lCV->getLine());

  ConstrainedNodeMec * n2 = new ConstrainedNodeMec("n2");
  addLink(n2);
  n2->setpFunction(new ConstantFunction<double(double)>(3e5));
  n2->addInFlow(lCV->getLine());

  getFrame("I")->enableOpenMBV(.025, 1);
  getFrame("ref")->enableOpenMBV(.025, 1);
  b->getFrame("C")->enableOpenMBV(.025, 1);
  b->getFrame("ref")->enableOpenMBV(.025, 1);
  lCV->enableOpenMBVFrames();
  lCV->enableOpenMBVArrows();
  lCV->enableOpenMBVBodies();
  n1->enableOpenMBVArrows(.01);
  n2->enableOpenMBVArrows(.01);

  setPlotFeatureRecursive("generalizedPosition",enabled);
  setPlotFeatureRecursive("generalizedVelocity",enabled);
  setPlotFeatureRecursive("generalizedRelativePosition",enabled);
  setPlotFeatureRecursive("generalizedRelativeVelocity",enabled);
  setPlotFeatureRecursive("generalizedForce",enabled);
  setPlotFeatureRecursive("signal",enabled);
}
