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

#ifndef _CONTACT_KINEMATICS_CIRCLE_PLANE_H_
#define _CONTACT_KINIMATICS_CIRCLE_PLANE_H_

#include "contact_kinematics.h"
#include "mbsim/mbsim_event.h"

namespace MBSim {

  class Circle;
  class Plane;

  /*! \brief pairing circle outer side to plane
   * \author Martin Foerg
   * \author Thorsten Schindler
   * \date 2009-07-28 pure virtual updates (Thorsten Schindler)
   */
  class ContactKinematicsCirclePlane : public ContactKinematics {
    public:
      /* INHERITED INTERFACE */
      virtual void assignContours(const std::vector<Contour*> &contour);
      virtual void updateg(double &g, std::vector<ContourFrame*> &cFrame, int index = 0);
      virtual void updatewb(fmatvec::Vec &wb, double g, std::vector<ContourFrame*> &cFrame) { throw MBSimError("(ContactKinematicsCirclePlane::updatewb): Not implemented!"); };
      /***************************************************/

    protected:
      /**
       * \brief contour index
       */
      int icircle, iplane;

      /**
       * \brief contour classes
       */
      Circle *circle;
      Plane *plane;
  };

}

#endif /* _CONTACT_KINEMATICS_CIRCLESOLID_PLANE_H_ */

