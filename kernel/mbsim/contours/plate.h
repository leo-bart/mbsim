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

#ifndef _RECTANGLE_H_
#define _RECTANGLE_H_

#include "mbsim/contours/plane.h"
#include "fmatvec/fmatvec.h"
#include "fmatvec/linear_algebra_double.h"
#include "fmatvec/linear_algebra.h"

#include <mbsim/utils/openmbv_utils.h>

namespace MBSim {

  /**
   *  \brief RigidContour Plate
   *  \date 2009-07-14 some comments (Bastian Esefeld)
   *  \date 2009-07-16 new file (Bastian Esefeld)
   */
  class Plate : public Plane {
    public:
      /**
       * \brief constructor
       * \param name of contour
       */
      Plate(const std::string &name="", Frame *R=0);

      Plate(const std::string &name, double yL, double zL, Frame *R=0);

      Plate(const std::string &name, double yL, double zL, double t, Frame *R=0);

      /* INHERITED INTERFACE OF ELEMENT */
      std::string getType() const {
        return "Plate";
      }
      virtual void init(InitStage stage);
      /**********************************/

      /* GETTER / SETTER */
      void setYLength(double l) {
        yLength = l;
      }
      void setZLength(double l) {
        zLength = l;
      }

      double getYLength() const {
        return yLength;
      }
      double getZLength() const {
        return zLength;
      }
      //get vertex of the plate, under reference frame of the plate
      const fmatvec::Vec3 & getA() const {
        return RrA;
      }
      const fmatvec::Vec3 & getB() const {
        return RrB;
      }
      const fmatvec::Vec3 & getC() const {
        return RrC;
      }
      const fmatvec::Vec3 & getD() const {
        return RrD;
      }

      /***************************************************//*new added part by ting 05.10.2012*/
      /*!
       * \brief if the point and plate are on the same plane by default, this function checks if a point lies in the plate;
       *        else this function checks if the projection of the point lies in the plate;
       * \param Point: position of the point
       * \return true: point in the plate
       */
      bool PointInPlate(const fmatvec::Vec3& Point); //need to be check again.......................................................

      /*!
       * \brief check if a point lies in the circle (point and circle are on the same plane)
       * \param Point position of the point
       * \param CenCir center of the circle
       * \param radius radius of the circle.
       * \return true: point in the circle, else false
       */
      bool PointInCircle(const fmatvec::Vec3& Point, const fmatvec::Vec3& CenCir, const double & radius);

      /*!
       * \brief search the closest point on the line segment to a circle
       * \param EndP1,EndP2: end points of the line segment; CenCir: center of the circle.
       */
      fmatvec::Vec3 Point_closest_toCircle_onLineseg(const fmatvec::Vec3& EndP1, const fmatvec::Vec3& EndP2, const fmatvec::Vec3& CenCir);

      /*!
       * \brief check if this plate intersect with a circle,
       * \param radius radius of the circle
       * \param CenCir center of the circle in world coordinates
       * \return true: this plate intersects the circle
       * \attention: the circle is the intersection circle between the sphere and the plane, thus the center of the
       * circle is different from the center of the sphere.
       */
      bool Intersect_Circle(const double & radius, const fmatvec::Vec3& CenCir);
      /***************************************************/

      BOOST_PARAMETER_MEMBER_FUNCTION( (void), enableOpenMBV, tag, (optional (diffuseColor,(const fmatvec::Vec3&),"[-1;1;1]")(transparency,(double),0))) { 
        OpenMBVCuboid ombv(fmatvec::Vec3(),diffuseColor,transparency);
        openMBVRigidBody=ombv.createOpenMBV(); 
      }

    protected:
      double yLength, zLength;
      //vertices under reference frame
      /*!
       * \brief vertex for y = limy/2, z = limz/2
       */
      fmatvec::Vec3 RrA;

      /*!
       * \brief vertex for y = -limy/2, z = limz/2
       */
      fmatvec::Vec3 RrB;

      /*!
       * \brief vertex for y = -limy/2, z = -limz/2
       */
      fmatvec::Vec3 RrC;

      /*!
       * \brief vertex for y = limy/2, z = -limz/2
       */
      fmatvec::Vec3 RrD;

    private:
      /*!
       * \brief set the verticies positions
       */
      void setVertices();

  };
}

#endif /* _AREA_H_ */
