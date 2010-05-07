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
 * Contact: mfoerg@users.berlios.de
 */

#ifndef _CONSTRAINT_H
#define _CONSTRAINT_H

#include "object.h"

namespace MBSim {

  class RigidBody;
  class Frame;

  /** 
   * \brief class for constraints between generalized coordinates of objects
   * \author Martin Foerg
   */
  class Constraint : public Object {
    private:

    public:
      Constraint(const std::string &name);
      void updateInverseKineticsJacobians(double t) {}
#ifdef HAVE_OPENMBVCPPINTERFACE
      virtual OpenMBV::Group* getOpenMBVGrp() {return 0;}
#endif
  };

  /** 
   * \brief example 1 for contraint 
   * \todo generalization of this class
   * \author Martin Foerg
   */
  class Constraint1 : public Constraint {
    private:
      RigidBody *bi, *bd1, *bd2;

      Frame *frame1,*frame2;
      int if1, if2;
      
      fmatvec::Mat d;

      fmatvec::Vec q12, u12;
      fmatvec::Mat J12;
      fmatvec::Vec j12;
      fmatvec::Mat J12t;

    public:
      Constraint1(const std::string &name, RigidBody* b0, RigidBody* b1, RigidBody* b2, Frame* frame1, Frame* frame2);

      void init(InitStage stage);
      void initz();

      fmatvec::Vec res(const fmatvec::Vec& q, const double& t);
      void setForceDirection(const fmatvec::Mat& d_) {d = d_;}

      void updateStateDependentVariables(double t); 
      void updateJacobians(double t); 
  };

  /** 
   * \brief example 2 for contraint 
   * \todo generalization of this class
   * \author Martin Foerg
   */
  class Constraint2 : public Constraint {
    private:
      std::vector<RigidBody*> bi;
      RigidBody *bd;
      std::vector<double> ratio;

//      std::vector<Function2<fmatvec::Vec, fmatvec::Vec, double>*> fd;
//      std::vector<Function2<fmatvec::Mat, fmatvec::Vec, double>*> fdJ;
//      std::vector<Function1<fmatvec::Vec,double>*> fdj;
//      std::vector<Function3<fmatvec::Mat, fmatvec::Vec, fmatvec::Vec, double>*> fdJd;
//      std::vector<Function1<fmatvec::Vec,double>*> fdjd;
    public:
      Constraint2(const std::string &name, RigidBody* body);

      void addDependency(RigidBody* body_, double ratio);

      void init(InitStage stage);

      void updateStateDependentVariables(double t);
      void updateJacobians(double t);
  };

  /** 
   * \brief example 3 for contraint 
   * \todo generalization of this class
   * \author Martin Foerg
   */
  class Constraint3 : public Constraint {
    private:
      RigidBody *bd;

    public:
      Constraint3(const std::string &name, RigidBody* body);

      void updateStateDependentVariables(double t);
      void updateJacobians(double t);
  };

  /** 
   * \brief example 4 for contraint 
   * \todo generalization of this class
   * \author Martin Foerg
   */
  class Constraint4 : public Constraint {
    private:
      RigidBody *bi, *bd1, *bd2;

      Frame *frame1,*frame2,*frame3;
      int if1, if2;
      
      fmatvec::Mat d;

      fmatvec::Vec x0;

    public:
      Constraint4(const std::string &name, RigidBody* b0, RigidBody* b1, RigidBody* b2, Frame* frame1, Frame* frame2, Frame* frame3);

      void init(InitStage stage);
      void initz();

      fmatvec::Vec res(const fmatvec::Vec& q, const double& t);
      void setForceDirection(const fmatvec::Mat& d_) {d = d_;}

      void updateStateDependentVariables(double t); 
      void updateJacobians(double t); 
  };

  /** 
   * \brief example 5 for contraint 
   * \todo generalization of this class
   * \author Martin Foerg
   */
  class Constraint5 : public Constraint {
    private:
      RigidBody *bi;
      std::vector<RigidBody*> bd1;
      std::vector<RigidBody*> bd2;
      std::vector<int> if1;
      std::vector<int> if2;

      Frame *frame1,*frame2;
      
      fmatvec::Mat dT;
      fmatvec::Mat dR;

      std::vector<fmatvec::Index> Iq1, Iq2, Iu1, Iu2;
      int nq, nu;
      fmatvec::Vec q, u;
      fmatvec::Mat J;
      fmatvec::Vec j;
      fmatvec::Mat JT, JR;
      fmatvec::Vec q0;

    public:
      Constraint5(const std::string &name, RigidBody* bi, std::vector<RigidBody*> bd1, std::vector<RigidBody*> bd2, Frame* frame1, Frame* frame2);

      void init(InitStage stage);
      void initz();

      void setForceDirection(const fmatvec::Mat& d_) {dT = d_;}
      void setMomentDirection(const fmatvec::Mat& d_) {dR = d_;}
      void  setq0(const fmatvec::Vec& q0_) {q0 = q0_;}

      fmatvec::Vec res(const fmatvec::Vec& q, const double& t);
      void updateStateDependentVariables(double t); 
      void updateJacobians(double t); 
  };

}

#endif
