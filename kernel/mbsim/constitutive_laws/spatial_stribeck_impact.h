/* Copyright (C) 2004-2014 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 */

#ifndef _SPATIAL_STRIBECK_IMPACT_H_
#define _SPATIAL_STRIBECK_IMPACT_H_

#include <mbsim/constitutive_laws/friction_impact_law.h>

namespace MBSim {

  /**
   * \brief spatial Stribeck friction force law on velocity level for constraint description
   * \author Thorsten Schindler
   * \date 2009-09-02 initial commit (Thorsten Schindler)
   * \todo high oscillations in normal relative velocity TODO
   */
  class SpatialStribeckImpact : public FrictionImpactLaw {
    public:
      /**
       * \brief constructor
       */
      SpatialStribeckImpact(Function<double(double)> *fmu_=NULL) : fmu(fmu_) {
        if(fmu) fmu->setParent(this);
      }

      /**
       * \brief destructor
       */
      virtual ~SpatialStribeckImpact() { if(fmu) delete fmu; fmu=0; }

      void init(Element::InitStage stage) {
        FrictionImpactLaw::init(stage);
        fmu->init(stage);
      }

      /* INHERITED INTERFACE */
      virtual fmatvec::Vec project(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Mat diff(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double r);
      virtual fmatvec::Vec solve(const fmatvec::SqrMat& G, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN);
      virtual bool isFulfilled(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double tolla, double tolgd);
      virtual int isSticking(const fmatvec::Vec& la, const fmatvec::Vec& gdn, const fmatvec::Vec& gda, double laN, double laTol, double gdTol);
      virtual int getFrictionDirections() { return 2; }
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual std::string getType() const { return "SpatialStribeckImpact"; }
      /***************************************************/

      double getFrictionCoefficient(double gd) { return (*fmu)(gd); }

      void setFrictionFunction(Function<double(double)> *fmu_) {
        fmu = fmu_;
        if(fmu) fmu->setParent(this);
      }

    protected:
      /**
       * friction coefficient function
       */
      Function<double(double)> *fmu;
  };

}

#endif
