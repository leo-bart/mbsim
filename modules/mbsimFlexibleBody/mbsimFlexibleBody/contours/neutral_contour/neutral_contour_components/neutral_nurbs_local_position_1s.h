/*
 * NeutralPosition1sNurbs.h
 *
 *  Created on: 24.10.2013
 *      Author: zwang
 */

#ifndef _NEUTRAL_NURBS_LOCAL_POSITION_1S_H_
#define _NEUTRAL_NURBS_LOCAL_POSITION_1S_H_

#include "neutral_nurbs_1s.h"

namespace MBSimFlexibleBody {
  
  class NeutralNurbsLocalPosition1s : public MBSimFlexibleBody::NeutralNurbs1s {
    public:
      NeutralNurbsLocalPosition1s(MBSim::Element* parent_, const fmatvec::VecInt & nodes, double nodeOffset_, double uMin_, double uMax_, int degU_, bool openStructure_);
      virtual ~NeutralNurbsLocalPosition1s();
      fmatvec::Vec3 evalLocalPosition(double s);
      virtual void update(MBSim::ContourFrame *frame);
    protected:
      virtual void buildNodelist();
  };

} /* namespace MBSimFlexibleBody */
#endif
