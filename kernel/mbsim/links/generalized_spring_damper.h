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

#ifndef _GENERALIZED_SPRING_DAMPER_H_
#define _GENERALIZED_SPRING_DAMPER_H_

#include "mbsim/links/dual_rigid_body_link.h"
#include "mbsim/functions/function.h"

#include "mbsim/utils/boost_parameters.h"
#include "mbsim/utils/openmbv_utils.h"

namespace MBSim {

  class GeneralizedSpringDamper : public DualRigidBodyLink {
    protected:
      Function<double(double,double)> *func;
      double l0;
    public:
      GeneralizedSpringDamper(const std::string &name="") : DualRigidBodyLink(name), func(NULL), l0(0) { }
      ~GeneralizedSpringDamper();

      void updateGeneralizedForces();

      bool isActive() const { return true; }
      bool gActiveChanged() { return false; }
      virtual bool isSingleValued() const { return true; }
      std::string getType() const { return "GeneralizedSpringDamper"; }
      void init(InitStage stage);

      /** \brief Set the function for the generalized force. */
      void setGeneralizedForceFunction(Function<double(double,double)> *func_) {
        func=func_;
        func->setParent(this);
        func->setName("GeneralizedFoce");
      }

      /** \brief Set unloaded generalized length. */
      void setGeneralizedUnloadedLength(double l0_) { l0 = l0_; }

      void initializeUsingXML(xercesc::DOMElement *element);
  };

}

#endif
