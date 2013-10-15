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

#ifndef _SPRINGDAMPER_H_
#define _SPRINGDAMPER_H_

#include "mbsim/link_mechanics.h"
#include <mbsim/frame.h>
#include "fmatvec/function.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
namespace OpenMBV {
  class CoilSpring;
}
#endif

namespace MBSim {

  /** \brief A spring damper force law.
   * This class connects two frames and applies a force in it, which depends in the
   * distance and relative velocity between the two frames.
   */
  class SpringDamper : public LinkMechanics {
    protected:
      double dist;
      fmatvec::Vec3 n;
      fmatvec::Function<double(double,double)> *func;
#ifdef HAVE_OPENMBVCPPINTERFACE
      OpenMBV::CoilSpring *coilspringOpenMBV;
#endif
    public:
      SpringDamper(const std::string &name="");
      void updateh(double, int i=0);
      void updateg(double);
      void updategd(double);

      /** \brief Connect the SpringDamper to frame1 and frame2 */
      void connect(Frame *frame1, Frame* frame2);

      /*INHERITED INTERFACE OF LINK*/
      bool isActive() const { return true; }
      bool gActiveChanged() { return false; }
      bool isSingleValued() const { return true; }
      std::string getType() const { return "SpringDamper"; }
      void init(InitStage stage);
      /*****************************/

      /** \brief Set function for the force calculation.
       * The first input parameter to that function is the distance g between frame2 and frame1.
       * The second input parameter to that function is the relative velocity gd between frame2 and frame1.
       * The return value of that function is used as the force of the SpringDamper.
       */
      void setForceFunction(fmatvec::Function<double(double,double)> *func_) { func=func_; }

      void plot(double t, double dt=1);
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

#ifdef HAVE_OPENMBVCPPINTERFACE
      /** \brief Visualise the SpringDamper using a OpenMBV::CoilSpring */
      void setOpenMBVCoilSpring(OpenMBV::CoilSpring *spring_) {coilspringOpenMBV=spring_;}

      /** \brief Visualize a force arrow acting on each of both connected frames */
      void setOpenMBVForceArrow(OpenMBV::Arrow *arrow) {
        std::vector<bool> which; which.resize(2, true);
        LinkMechanics::setOpenMBVForceArrow(arrow, which);
      }
#endif
    private:
      std::string saved_ref1, saved_ref2;
  };

  /** \brief A spring damper force law.
   * This class connects two frames and applies a force in it, which depends in the
   * distance and relative velocity between the two frames.
   */
  class DirectionalSpringDamper : public LinkMechanics {
    protected:
      double dist;
      fmatvec::Function<double(double,double)> *func;
      Frame *refFrame;
      fmatvec::Vec3 forceDir, WforceDir, WrP0P1;
      Frame C;
#ifdef HAVE_OPENMBVCPPINTERFACE
      OpenMBV::CoilSpring *coilspringOpenMBV;
#endif
    public:
      DirectionalSpringDamper(const std::string &name="");
      void updateh(double, int i=0);
      void updateg(double);
      void updategd(double);

      /** \brief Connect the SpringDamper to frame1 and frame2 */
      void connect(Frame *frame1, Frame* frame2);

      /*INHERITED INTERFACE OF LINK*/
      bool isActive() const { return true; }
      bool gActiveChanged() { return false; }
      bool isSingleValued() const { return true; }
      std::string getType() const { return "DirectionalSpringDamper"; }
      void init(InitStage stage);
      /*****************************/

      /** \brief Set function for the force calculation.
       * The first input parameter to that function is the distance g between frame2 and frame1.
       * The second input parameter to that function is the relative velocity gd between frame2 and frame1.
       * The return value of that function is used as the force of the SpringDamper.
       */
      void setForceFunction(fmatvec::Function<double(double,double)> *func_) { func=func_; }

      /**
       * \param local force direction represented in first frame
       */
      void setForceDirection(const fmatvec::Vec3 &dir) { forceDir=dir/nrm2(dir); }

      void plot(double t, double dt=1);
      void initializeUsingXML(MBXMLUtils::TiXmlElement *element);

#ifdef HAVE_OPENMBVCPPINTERFACE
      /** \brief Visualise the SpringDamper using a OpenMBV::CoilSpring */
      void setOpenMBVCoilSpring(OpenMBV::CoilSpring *spring_) {coilspringOpenMBV=spring_;}

      /** \brief Visualize a force arrow acting on each of both connected frames */
      void setOpenMBVForceArrow(OpenMBV::Arrow *arrow) {
        std::vector<bool> which; which.resize(2, true);
        LinkMechanics::setOpenMBVForceArrow(arrow, which);
      }
#endif
    private:
      std::string saved_ref1, saved_ref2;
  };


}

#endif /* _SPRINGDAMPER_H_ */

