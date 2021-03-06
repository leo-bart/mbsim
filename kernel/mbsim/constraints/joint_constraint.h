/* Copyright (C) 2004-2009 MBSim Development Team
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

#ifndef _JOINT_CONSTRAINT_H
#define _JOINT_CONSTRAINT_H

#include "mechanical_constraint.h"
#include "mbsim/functions/function.h"
#include "mbsim/frames/floating_relative_frame.h"
#include "mbsim/utils/index.h"

namespace MBSim {

  class RigidBody;
  class Frame;

  /** 
   * \brief Joint contraint 
   * \author Martin Foerg
   * 2011-08-04 XML Interface added (Markus Schneider)
   */
  class JointConstraint : public MechanicalConstraint {
    public:
      JointConstraint(const std::string &name="");

      void init(InitStage stage);

      void resetUpToDate();

      void connect(Frame* frame1_, Frame* frame2_) { frame1 = frame1_; frame2 = frame2_; }
      void addDependentRigidBodyOnFirstSide(RigidBody* bd) { bd1.push_back(bd); }
      void addDependentRigidBodyOnSecondSide(RigidBody* bd) { bd2.push_back(bd); }
      void setIndependentRigidBody(RigidBody* bi_) { bi.resize(1); bi[0] = bi_; }
      void addIndependentRigidBody(RigidBody* bi_) { bi.push_back(bi_); }

      virtual void setUpInverseKinetics();
      void setForceDirection(const fmatvec::Mat3xV& d_);
      void setMomentDirection(const fmatvec::Mat3xV& d_);

      /** \brief The frame of reference ID for the force/moment direction vectors.
       * If ID=0 (default) the first frame, if ID=1 the second frame is used.
       */
      void setFrameOfReferenceID(int ID) { refFrameID=ID; }

      void updatePositions(Frame *frame_);
      void updateGeneralizedCoordinates();
      void updateGeneralizedJacobians(int j=0);
      virtual void initializeUsingXML(xercesc::DOMElement *element);

      virtual std::string getType() const { return "JointConstraint"; }

      void setInitialGuess(const fmatvec::VecV &q0_) { q0 = q0_; }

    private:
      class Residuum : public Function<fmatvec::Vec(fmatvec::Vec)> {
        private:
          std::vector<RigidBody*> body1, body2;
          fmatvec::Mat3xV forceDir, momentDir;
          Frame *frame1, *frame2, *refFrame;
          std::vector<Frame*> i1,i2;
        public:
          Residuum(std::vector<RigidBody*> body1_, std::vector<RigidBody*> body2_, const fmatvec::Mat3xV &forceDir_, const fmatvec::Mat3xV &momentDir_, Frame *frame1_, Frame *frame2_, Frame *refFrame, std::vector<Frame*> i1_, std::vector<Frame*> i2_);
          fmatvec::Vec operator()(const fmatvec::Vec &x);
      };
      std::vector<RigidBody*> bd1, bd2, bi;
      std::vector<Frame*> if1, if2;

      Frame *frame1, *frame2;

      /**
       * \brief frame of reference the force is defined in
       */
      Frame *refFrame;
      Index refFrameID;

      FloatingRelativeFrame C;

      fmatvec::Mat3xV dT, dR, forceDir, momentDir;

      std::vector<fmatvec::RangeV> Iq1, Iq2, Iu1, Iu2, Ih1, Ih2;
      int nq, nu, nh;
      fmatvec::Vec q, q0;
      fmatvec::Mat JT, JR;
      fmatvec::Mat3xV Js;

      std::string saved_ref1, saved_ref2;
      std::vector<std::string> saved_RigidBodyFirstSide, saved_RigidBodySecondSide, saved_IndependentBody;
  };

}

#endif
