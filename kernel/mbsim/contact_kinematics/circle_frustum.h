/* Copyright (C) 2004-2009 MBSim Development Team
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

#ifndef CIRCLE_FRUSTUM_H_
#define CIRCLE_FRUSTUM_H_

#include "mbsim/contact_kinematics/contact_kinematics.h"
#include "mbsim/mbsim_event.h"

namespace MBSim {

  class Circle;
  class Frustum;

  /*!
   * \brief contact kinematics for unilateral contact between circle and frustum with at most one contact point
   * \author Thorsten Schindler
   * \author Bastian Esefeld
   * \date 2009-07-13 initial commit kernel_dev
   * \date 2009-09-02 performance improvement
   *
   * \theoretical background: "Spatial Dynamics of Pushbelt CVTs" (Schindler 2010 p.34 ff)
   */
  class ContactKinematicsCircleFrustum : public ContactKinematics {
    public:
      /*!
       * \brief constructor
       * \default no debugging
       * \default no warnings
       * \default global search
       */
      ContactKinematicsCircleFrustum();

      /*!
       * \brief destructor
       */
      virtual ~ContactKinematicsCircleFrustum();

      /* INHERITED INTERFACE OF CONTACTKINEAMTICS */
      void assignContours(const std::vector<Contour*> &contour);
      virtual void updateg(double& g, std::vector<ContourFrame*> &cFrame, int index = 0);
      virtual void updatewb(fmatvec::Vec& wb, double g, std::vector<ContourFrame*> &cFrame) { throw MBSimError("(ContactKinematicsCircleFrustum::updatewb): not implemented!"); }
      /***************************************************/

      /* GETTER / SETTER */
      void setDebug(bool DEBUG_);
      void setWarnLevel(int warnLevel_);
      void setLocalSearch(bool LOCALSEARCH_);
      /***************************************************/

    protected:
      /**
       * \brief contour index
       */
      int icircle, ifrustum;

      /**
       * \brief contour classes
       */
      Frustum *frustum;
      Circle *circle;

      /**
       * \brief debug information during calculation?
       */
      bool DEBUG;

      /**
       * \brief warnings during calculation?
       */
      int warnLevel;

      /**
       * \brief local contact search?
       */
      bool LOCALSEARCH;

      fmatvec::Vec2 zeta;
  };

  inline void ContactKinematicsCircleFrustum::setDebug(bool DEBUG_) { DEBUG=DEBUG_; }
  inline void ContactKinematicsCircleFrustum::setWarnLevel(int warnLevel_) { warnLevel=warnLevel_; }
  inline void ContactKinematicsCircleFrustum::setLocalSearch(bool LOCALSEARCH_) { LOCALSEARCH=LOCALSEARCH_; }

}

#endif /* CIRCLE_FRUSTUM_H_ */

