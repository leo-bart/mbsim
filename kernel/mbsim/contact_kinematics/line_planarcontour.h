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

#ifndef _CONTACT_KINEMATICS_LINE_PLANAR_CONTOUR_H_
#define _CONTACT_KINEMATICS_LINE_PLANAR_CONTOUR_H_

#include "contact_kinematics.h"
#include "mbsim/mbsim_event.h"

namespace MBSim {

  class Line;
  class Contour;
  class FuncPairPlanarContourLine;

  /**
   * \brief pairing Line to PlanarContour
   * \author Martin Foerg
   * \date 2009-07-28 pure virtual updates (Thorsten Schindler)
   */
  class ContactKinematicsLinePlanarContour : public ContactKinematics {
    public:
      /**
       * \brief constructor
       */
      ContactKinematicsLinePlanarContour() {}

      /**
       * \brief destructor
       */
      virtual ~ContactKinematicsLinePlanarContour();
      
      /* INHERITED INTERFACE */
      virtual void assignContours(const std::vector<Contour*> &contour);
      virtual void updateg(double &g, std::vector<ContourFrame*> &cFrame, int index = 0);
      virtual void updatewb(fmatvec::Vec &wb, double g, std::vector<ContourFrame*> &cFrame) { throw MBSimError("(ContactKinematicsLinePlanarContour::updatewb): Not implemented!"); };
      /***************************************************/

    private:
      /**
       * \brief contour index
       */
      int iline, icontour;

      /**
       * \brief contour classes
       */
      Line *line;
      Contour *contour1s;

      /**
       * \brief root function
       */
      FuncPairPlanarContourLine *func;
  };

}

#endif /* _CONTACT_KINEMATICS_LINE_CONTOUR1S_H_ */

