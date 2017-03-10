// wrap MBSim with directors (and all protected class data)
%module(directors="1", allprotected="1") _mbsim_part300


// include the general mbsim SWIG configuration (used by all MBSim modules)
%import fmatvec.i
%include "mbsim_include.i"
%import mbsim.i
%import _mbsim_part100.i
%import _mbsim_part200.i
%import _mbsim_part250.i

// includes needed in the generated swig c++ code
%{
#include <config.h> // to use consistent preprocessor defines
namespace MBSim {
  class Body;
  class KinematicCoordinatesObserver;
  class Circle;
  class CompoundContour;
  class Constraint;
  class MechanicalConstraint;
  class Contact;
  class ContactPolyfun;
  class Contour;
  class ContourFrame;
  class ContourInterpolation;
  class ContourLink;
  class ContourQuad;
  class CriteriaFunction;
  class Cuboid;
  class DampingFunction;
  class DirectionalSpringDamper;
  template<typename Sig> class DistanceFunction;
  class DualRigidBodyLink;
  class DynamicSystem;
  class Edge;
  class Environment;
  class FixedContourFrame;
  class FixedRelativeFrame;
  class FloatingContourFrame;
  class FixedFrameLink;
  class FloatingFrameLink;
  class FloatingRelativeContourFrame;
  class FloatingRelativeFrame;
  class FrameLink;
  class Frustum;
  class GeneralizedAccelerationConstraint;
  class GeneralizedAccelerationExcitation;
  class GeneralizedConstraint;
  class GeneralizedDualConstraint;
  class GeneralizedFriction;
  class GeneralizedKinematicExcitation;
  class GeneralizedPositionConstraint;
  class GeneralizedPositionExcitation;
  class GeneralizedSpringDamper;
  class GeneralizedVelocityConstraint;
  class GeneralizedVelocityExcitation;
  class GlobalCriteriaFunction;
  class GlobalResidualCriteriaFunction;
  class GlobalShiftCriteriaFunction;
  class Graph;
  class Group;
  class InverseKineticsJoint;
  class IsotropicRotationalSpringDamper;
  class Joint;
  class JointConstraint;
  class KinematicsObserver;
  class KineticExcitation;
  class Line;
  class LineSegment;
  class Link;
  class MechanicalLink;
  class LocalCriteriaFunction;
  class LocalResidualCriteriaFunction;
  class LocalShiftCriteriaFunction;
  class MBSimEnvironment;
  class NewtonJacobianFunction;
  class NumericalNewtonJacobianFunction;
  class Object;
  class Observer;
  class PlanarContour;
  class PlanarFrustum;
  class Plane;
  class PlaneWithFrustum;
  class Plate;
  class Point;
  class PolynomialFrustum;
  class RelativeKinematicsObserver;
  class RigidBody;
  class RigidBodyGroupObserver;
  class RigidBodyLink;
  class RigidContour;
  class Room;
  class SpatialContour;
  class Sphere;
  class SpringDamper;
  class StandardDampingFunction;
  class FuncPairPlanarContourPoint;
}
#include "mbsim/constitutive_laws/friction_force_law.h"
#include "mbsim/constitutive_laws/planar_coulomb_friction.h"
#include "mbsim/constitutive_laws/planar_stribeck_friction.h"
#include "mbsim/constitutive_laws/regularized_planar_friction.h"
#include "mbsim/constitutive_laws/regularized_spatial_friction.h"
#include "mbsim/constitutive_laws/spatial_coulomb_friction.h"
#include "mbsim/constitutive_laws/spatial_stribeck_friction.h"
#include "mbsim/constitutive_laws/friction_impact_law.h"
#include "mbsim/constitutive_laws/planar_coulomb_impact.h"
#include "mbsim/constitutive_laws/planar_stribeck_impact.h"
#include "mbsim/constitutive_laws/spatial_coulomb_impact.h"
#include "mbsim/constitutive_laws/spatial_stribeck_impact.h"
#include "mbsim/constitutive_laws/generalized_force_law.h"
#include "mbsim/constitutive_laws/bilateral_constraint.h"
#include "mbsim/constitutive_laws/regularized_bilateral_constraint.h"
#include "mbsim/constitutive_laws/regularized_unilateral_constraint.h"
#include "mbsim/constitutive_laws/unilateral_constraint.h"
#include "mbsim/constitutive_laws/generalized_impact_law.h"
#include "mbsim/constitutive_laws/bilateral_impact.h"
#include "mbsim/constitutive_laws/unilateral_newton_impact.h"
#include "mbsim/contact_kinematics/contact_kinematics.h"
#include "mbsim/contact_kinematics/circle_circle.h"
#include "mbsim/contact_kinematics/circle_extrusion.h"
#include "mbsim/contact_kinematics/circle_frustum.h"
#include "mbsim/contact_kinematics/circle_line.h"
#include "mbsim/contact_kinematics/circle_linesegment.h"
#include "mbsim/contact_kinematics/circle_planarcontour.h"
#include "mbsim/contact_kinematics/circle_planarfrustum.h"
#include "mbsim/contact_kinematics/circle_plane.h"
#include "mbsim/contact_kinematics/compoundcontour_compoundcontour.h"
#include "mbsim/contact_kinematics/compoundcontour_contour.h"
#include "mbsim/contact_kinematics/edge_edge.h"
#include "mbsim/contact_kinematics/line_planarcontour.h"
#include "mbsim/contact_kinematics/plate_polynomialfrustum.h"
#include "mbsim/contact_kinematics/point_circle.h"
#include "mbsim/contact_kinematics/point_contourinterpolation.h"
#include "mbsim/contact_kinematics/point_extrusion.h"
#include "mbsim/contact_kinematics/point_frustum.h"
#include "mbsim/contact_kinematics/point_line.h"
#include "mbsim/contact_kinematics/point_linesegment.h"
#include "mbsim/contact_kinematics/point_planarcontour.h"
#include "mbsim/contact_kinematics/point_plane.h"
#include "mbsim/contact_kinematics/point_planewithfrustum.h"
#include "mbsim/contact_kinematics/point_plate.h"
#include "mbsim/contact_kinematics/point_polynomialfrustum.h"
#include "mbsim/contact_kinematics/point_spatialcontour.h"
#include "mbsim/contact_kinematics/point_sphere.h"
#include "mbsim/contact_kinematics/sphere_frustum.h"
#include "mbsim/contact_kinematics/sphere_plane.h"
#include "mbsim/contact_kinematics/sphere_plate.h"
#include "mbsim/contact_kinematics/sphere_polynomialfrustum.h"
#include "mbsim/contact_kinematics/sphere_sphere.h"
#include "mbsim/numerics/nonlinear_algebra/multi_dimensional_newton_method.h"
#include "mbsim/modelling_interface.h"
#include "mbsim/utils/colors.h"
using namespace MBSim; // SWIGs namespace handling seems to be buggy -> this fixes this
using namespace fmatvec; // SWIGs namespace handling seems to be buggy -> this fixes this
%}



// wrap the following classes
%include "mbsim/constitutive_laws/friction_force_law.h"
%include "mbsim/constitutive_laws/planar_coulomb_friction.h"
%include "mbsim/constitutive_laws/planar_stribeck_friction.h"
%include "mbsim/constitutive_laws/regularized_planar_friction.h"
%include "mbsim/constitutive_laws/regularized_spatial_friction.h"
%include "mbsim/constitutive_laws/spatial_coulomb_friction.h"
%include "mbsim/constitutive_laws/spatial_stribeck_friction.h"
%include "mbsim/constitutive_laws/friction_impact_law.h"
%include "mbsim/constitutive_laws/planar_coulomb_impact.h"
%include "mbsim/constitutive_laws/planar_stribeck_impact.h"
%include "mbsim/constitutive_laws/spatial_coulomb_impact.h"
%include "mbsim/constitutive_laws/spatial_stribeck_impact.h"
%include "mbsim/constitutive_laws/generalized_force_law.h"
%include "mbsim/constitutive_laws/bilateral_constraint.h"
%include "mbsim/constitutive_laws/regularized_bilateral_constraint.h"
%include "mbsim/constitutive_laws/regularized_unilateral_constraint.h"
%include "mbsim/constitutive_laws/unilateral_constraint.h"
%include "mbsim/constitutive_laws/generalized_impact_law.h"
%include "mbsim/constitutive_laws/bilateral_impact.h"
%include "mbsim/constitutive_laws/unilateral_newton_impact.h"
%include "mbsim/contact_kinematics/contact_kinematics.h"
%include "mbsim/contact_kinematics/circle_circle.h"
%include "mbsim/contact_kinematics/circle_extrusion.h"
%include "mbsim/contact_kinematics/circle_frustum.h"
%include "mbsim/contact_kinematics/circle_line.h"
%include "mbsim/contact_kinematics/circle_linesegment.h"
%include "mbsim/contact_kinematics/circle_planarcontour.h"
%include "mbsim/contact_kinematics/circle_planarfrustum.h"
%include "mbsim/contact_kinematics/circle_plane.h"
%include "mbsim/contact_kinematics/compoundcontour_compoundcontour.h"
%include "mbsim/contact_kinematics/compoundcontour_contour.h"
%include "mbsim/contact_kinematics/edge_edge.h"
%include "mbsim/contact_kinematics/line_planarcontour.h"
%include "mbsim/contact_kinematics/plate_polynomialfrustum.h"
%include "mbsim/contact_kinematics/point_circle.h"
%include "mbsim/contact_kinematics/point_contourinterpolation.h"
%include "mbsim/contact_kinematics/point_extrusion.h"
%include "mbsim/contact_kinematics/point_frustum.h"
%include "mbsim/contact_kinematics/point_line.h"
%include "mbsim/contact_kinematics/point_linesegment.h"
%include "mbsim/contact_kinematics/point_planarcontour.h"
%include "mbsim/contact_kinematics/point_plane.h"
%include "mbsim/contact_kinematics/point_planewithfrustum.h"
%include "mbsim/contact_kinematics/point_plate.h"
%include "mbsim/contact_kinematics/point_polynomialfrustum.h"
%include "mbsim/contact_kinematics/point_spatialcontour.h"
%include "mbsim/contact_kinematics/point_sphere.h"
%include "mbsim/contact_kinematics/sphere_frustum.h"
%include "mbsim/contact_kinematics/sphere_plane.h"
%include "mbsim/contact_kinematics/sphere_plate.h"
%include "mbsim/contact_kinematics/sphere_polynomialfrustum.h"
%include "mbsim/contact_kinematics/sphere_sphere.h"
%include "mbsim/numerics/nonlinear_algebra/multi_dimensional_newton_method.h"
%include "mbsim/modelling_interface.h"
%include "mbsim/utils/colors.h"
