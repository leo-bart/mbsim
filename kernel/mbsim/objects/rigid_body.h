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
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _RIGID_BODY_H_
#define _RIGID_BODY_H_

#include "mbsim/objects/body.h"
#include "mbsim/frames/frame.h"
#include "mbsim/functions/time_dependent_function.h"
#include "mbsim/functions/state_dependent_function.h"

namespace MBSim {

  class Frame;
  class RigidContour;
  class FixedRelativeFrame;
  class Constraint;
  class InverseKineticsJoint;

  /**
   * \brief rigid bodies with arbitrary kinematics 
   * \author Martin Foerg
   * \date 2009-04-08 some comments (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-12-14 revised inverse kinetics (Martin Foerg)
   * \date 2010-04-24 class can handle constraints on generalized coordinates (Martin Foerg) 
   * \date 2010-06-20 add getter for Kinematics; revision on doxygen comments (Roland Zander)
   * \todo kinetic energy TODO
   * \todo Euler parameter TODO
   * \todo check if inertial system for performance TODO
   *
   * rigid bodies have a predefined canonic Frame 'C' in their centre of gravity 
   */
  class RigidBody : public Body {
    public:
      /**
       * \brief constructor
       * \param name name of rigid body
       */
      RigidBody(const std::string &name="");

      /**
       * \brief destructor
       */
      virtual ~RigidBody();

      void addDependency(Constraint* constraint_);

      void updateqd();
      void updateT();
      void updateh(int j=0);
      void updateM() { (this->*updateM_)(); }
      void updateInertiaTensor();
      void updateGeneralizedPositions();
      void updateGeneralizedVelocities();
      void updateDerivativeOfGeneralizedPositions();
      void updateGeneralizedAccelerations();
      void updateGeneralizedJacobians(int j=0);
      void updatePositions();
      void updateVelocities();
      void updateJacobians();
      void updateGyroscopicAccelerations();
      void updatePositions(Frame *frame);
      void updateVelocities(Frame *frame);
      void updateAccelerations(Frame *frame);
      void updateJacobians(Frame *frame, int j=0) { (this->*updateJacobians_[j])(frame); }
      void updateGyroscopicAccelerations(Frame *frame);
      void updateJacobians0(Frame *frame);
      void updateJacobians1(Frame *frame) { }
      void updateJacobians2(Frame *frame);

      virtual void calcSize();
      virtual void calcqSize();
      virtual void calcuSize(int j=0);
      void sethSize(int hSize_, int i=0);
      void sethInd(int hInd_, int i=0);

      /* INHERITED INTERFACE OF OBJECT */
      virtual void init(InitStage stage);
      virtual void updateLLM() { (this->*updateLLM_)(); }
      virtual void setUpInverseKinetics();
      /*****************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "RigidBody"; }
      virtual void plot();
      /*****************************************************/

      /* GETTER / SETTER */

      // NOTE: we can not use a overloaded setTranslation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for genral translational motion
       * \param fPrPK translational kinematic description
       */
      void setGeneralTranslation(Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) {
        fPrPK = fPrPK_;
        fPrPK->setParent(this);
        fPrPK->setName("GeneralTranslation");
      }
      /*!
       * \brief set Kinematic for only time dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setTimeDependentTranslation(Function<fmatvec::Vec3(double)> *fPrPK_) {
        setGeneralTranslation(new TimeDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      /*!
       * \brief set Kinematic for only state dependent translational motion
       * \param fPrPK translational kinematic description
       */
      void setStateDependentTranslation(Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) {
        setGeneralTranslation(new StateDependentFunction<fmatvec::Vec3>(fPrPK_));
      }
      void setTranslation(Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK_) { setGeneralTranslation(fPrPK_); }
      void setTranslation(Function<fmatvec::Vec3(double)> *fPrPK_) { setTimeDependentTranslation(fPrPK_); }
      void setTranslation(Function<fmatvec::Vec3(fmatvec::VecV)> *fPrPK_) { setStateDependentTranslation(fPrPK_); }

      // NOTE: we can not use a overloaded setRotation here due to restrictions in XML but define them for convinience in c++
      /*!
       * \brief set Kinematic for general rotational motion
       * \param fAPK rotational kinematic description
       */
      void setGeneralRotation(Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_) {
        fAPK = fAPK_;
        fAPK->setParent(this);
        fAPK->setName("GeneralRotation");
      }
      /*!
       * \brief set Kinematic for only time dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setTimeDependentRotation(Function<fmatvec::RotMat3(double)>* fAPK_) { setGeneralRotation(new TimeDependentFunction<fmatvec::RotMat3>(fAPK_)); }
      /*!
       * \brief set Kinematic for only state dependent rotational motion
       * \param fAPK rotational kinematic description
       */
      void setStateDependentRotation(Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_) { setGeneralRotation(new StateDependentFunction<fmatvec::RotMat3>(fAPK_)); }
      void setRotation(Function<fmatvec::RotMat3(fmatvec::VecV, double)>* fAPK_) { setGeneralRotation(fAPK_); }
      void setRotation(Function<fmatvec::RotMat3(double)>* fAPK_) { setTimeDependentRotation(fAPK_); }
      void setRotation(Function<fmatvec::RotMat3(fmatvec::VecV)>* fAPK_) { setStateDependentRotation(fAPK_); }

      void setTranslationDependentRotation(bool dep) { translationDependentRotation = dep; }
      void setCoordinateTransformationForRotation(bool ct) { coordinateTransformation = ct; }
      void setBodyFixedRepresentationOfAngularVelocity(bool bf) { bodyFixedRepresentationOfAngularVelocity = bf; }

      /*!
       * \brief get Kinematic for translational motion
       * \return translational kinematic description
       */
      Function<fmatvec::Vec3(fmatvec::VecV, double)>* getTranslation() { return fPrPK; }
      /*!
       * \brief get Kinematic for rotational motion
       * \return rotational kinematic description
       */
      Function<fmatvec::RotMat3(fmatvec::VecV, double)>* getRotation() { return fAPK; }

      void setMass(double m_) { m = m_; }
      double getMass() const { return m; }
      Frame* getFrameForKinematics() { return &Z; };
      FixedRelativeFrame* getFrameC() { return C; };

      const fmatvec::Vec3& evalGlobalRelativePosition() { if(updPos) updatePositions(); return WrPK; }
      const fmatvec::Vec3& evalGlobalRelativeVelocity() { if(updVel) updateVelocities(); return WvPKrel; }
      const fmatvec::Vec3& evalGlobalRelativeAngularVelocity() { if(updVel) updateVelocities(); return WomPK; }
      const fmatvec::SymMat3& evalGlobalInertiaTensor() { if(updWTS) updateInertiaTensor(); return WThetaS; }
      const fmatvec::Mat3xV& evalPJTT() { if(updPJ) updateJacobians(); return PJTT; }
      const fmatvec::Mat3xV& evalPJRR() { if(updPJ) updateJacobians(); return PJRR; }
      const fmatvec::Vec3& evalPjhT() { if(updPJ) updateJacobians(); return PjhT; }
      const fmatvec::Vec3& evalPjhR() { if(updPJ) updateJacobians(); return PjhR; }
      const fmatvec::Vec3& evalPjbT() { if(updPjb) updateGyroscopicAccelerations(); return PjbT; }
      const fmatvec::Vec3& evalPjbR() { if(updPjb) updateGyroscopicAccelerations(); return PjbR; }

      /**
       * \param RThetaR  inertia tensor
       * \param frame optional reference Frame of inertia tensor, otherwise cog-Frame will be used as reference
       */
      void setInertiaTensor(const fmatvec::SymMat3& RThetaR) { SThetaS = RThetaR; }

      const fmatvec::SymMat3& getInertiaTensor() const {return SThetaS;}
      fmatvec::SymMat3& getInertiaTensor() {return SThetaS;}

      void addFrame(FixedRelativeFrame *frame); 
      void addContour(RigidContour *contour);

      /**
       * \param frame Frame to be used for kinematical description depending on reference Frame and generalised positions / velocities
       */
      void setFrameForKinematics(Frame *frame);

      void setFrameForInertiaTensor(Frame *frame);

      void setOpenMBVRigidBody(const std::shared_ptr<OpenMBV::RigidBody> &body);
      void setOpenMBVFrameOfReference(Frame * frame) {openMBVFrame=frame; }
      const Frame* getOpenMBVFrameOfReference() const {return openMBVFrame; }

       virtual void initializeUsingXML(xercesc::DOMElement *element);

      fmatvec::MatV& getJRel(int i=0, bool check=true) { assert((not check) or (not updGJ)); return JRel[i]; }
      fmatvec::VecV& getjRel(bool check=true) { assert((not check) or (not updGJ)); return jRel; }
      void setqRel(const fmatvec::VecV &q);
      void setuRel(const fmatvec::VecV &u);
      void setJRel(const fmatvec::MatV &J);
      void setjRel(const fmatvec::VecV &j);

      bool transformCoordinates() const { return fTR!=NULL; }

      void resetUpToDate();
      void resetPositionsUpToDate();
      void resetVelocitiesUpToDate();
      void resetJacobiansUpToDate();
      void resetGyroscopicAccelerationsUpToDate();
      const fmatvec::VecV& evalqTRel() { if(updq) updateGeneralizedPositions(); return qTRel; }
      const fmatvec::VecV& evalqRRel() { if(updq) updateGeneralizedPositions(); return qRRel; }
      const fmatvec::VecV& evaluTRel() { if(updu) updateGeneralizedVelocities(); return uTRel; }
      const fmatvec::VecV& evaluRRel() { if(updu) updateGeneralizedVelocities(); return uRRel; }
      const fmatvec::VecV& evalqdTRel() { if(updqd) updateDerivativeOfGeneralizedPositions(); return qdTRel; }
      const fmatvec::VecV& evalqdRRel() { if(updqd) updateDerivativeOfGeneralizedPositions(); return qdRRel; }
      const fmatvec::MatV& evalJRel(int j=0) { if(updGJ) updateGeneralizedJacobians(); return JRel[j]; }
      const fmatvec::VecV& evaljRel() { if(updGJ) updateGeneralizedJacobians(); return jRel; }

      void setUpdateByReference(bool updateByReference_) { updateByReference = updateByReference_; }

      InverseKineticsJoint* getJoint() { return joint; }

    protected:
      /**
       * \brief mass
       */
      double m;

      /**
       * \brief inertia tensor with respect to centre of gravity in centre of gravity and world Frame
       */
      fmatvec::SymMat3 SThetaS, WThetaS;

      FixedRelativeFrame *K;

      /**
       * \brief TODO
       */
      fmatvec::SymMat Mbuf;

      /**
       * \brief boolean to use body fixed Frame for rotation
       */
      bool coordinateTransformation;

      fmatvec::Vec3 PjhT, PjhR, PjbT, PjbR;

      /**
       * \brief rotation matrix from kinematic Frame to parent Frame
       */
      fmatvec::SqrMat3 APK;

      /**
       * \brief translation from parent to kinematic Frame in parent and world system
       */
      fmatvec::Vec3 PrPK, WrPK;

      /**
       * \brief translational and angular velocity from parent to kinematic Frame in world system
       */
      fmatvec::Vec3 WvPKrel, WomPK;

      Function<fmatvec::MatV(fmatvec::VecV)> *fTR;

      /**
       * \brief translation from parent Frame to kinematic Frame in parent system
       */
      Function<fmatvec::Vec3(fmatvec::VecV, double)> *fPrPK;

      /**
       * \brief rotation from kinematic Frame to parent Frame
       */
      Function<fmatvec::RotMat3(fmatvec::VecV, double)> *fAPK;

      /**
       * \brief function pointer to update mass matrix
       */
      void (RigidBody::*updateM_)();

      /**
       * \brief update constant mass matrix
       */
      void updateMConst();

      /**
       * \brief update time dependend mass matrix
       */
      void updateMNotConst();

      /**
       * \brief function pointer for Cholesky decomposition of mass matrix
       */
      void (RigidBody::*updateLLM_)();

      /**
       * \brief Cholesky decomposition of constant mass matrix
       */
      void updateLLMConst() { }

      /**
       * \brief Cholesky decomposition of time dependent mass matrix
       */
      void updateLLMNotConst() { Object::updateLLM(); }

#ifndef SWIG
      void (RigidBody::*updateJacobians_[3])(Frame *frame);
#endif

      /** a pointer to Frame "C" */
      FixedRelativeFrame *C;

      fmatvec::MatV JRel[2];
      fmatvec::VecV jRel;

      fmatvec::VecV qTRel, qRRel, uTRel, uRRel, qdTRel, qdRRel;
      fmatvec::Mat3xV PJTT, PJRR;

      Constraint *constraint;

      Frame *frameForJacobianOfRotation;

      Frame *frameForInertiaTensor;

      fmatvec::Range<fmatvec::Var,fmatvec::Var> iqT, iqR, iuT, iuR;

      bool translationDependentRotation, constJT, constJR, constjT, constjR;

      bool updPjb, updGJ, updWTS, updateByReference;

      Frame Z;

      bool bodyFixedRepresentationOfAngularVelocity;
      
      InverseKineticsJoint *joint;

    private:
      /**
       * \brief Frame of reference for drawing openMBVBody
       */
      Frame * openMBVFrame;
  };

}

#endif
