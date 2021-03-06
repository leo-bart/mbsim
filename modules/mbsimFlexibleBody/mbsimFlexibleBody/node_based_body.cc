/* Copyright (C) 2004-2016 MBSim Development Team
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

#include <config.h>
#include "node_based_body.h"
#include <mbsimFlexibleBody/frames/node_frame.h>
#include <mbsim/mbsim_event.h>

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimFlexibleBody {

  NodeBasedBody::NodeBasedBody(const string &name) : Body(name) { }

  void NodeBasedBody::updatePositions(NodeFrame* frame) {
    THROW_MBSIMERROR("(NodeBasedBody::updatePositions): Not implemented.");
  }

  void NodeBasedBody::updateVelocities(NodeFrame* frame) {
    THROW_MBSIMERROR("(NodeBasedBody::updateVelocities): Not implemented.");
  }

  void NodeBasedBody::updateAccelerations(NodeFrame* frame) {
    THROW_MBSIMERROR("(NodeBasedBody::updateAccelerations): Not implemented.");
  }

  void NodeBasedBody::updateJacobians(NodeFrame* frame, int j) {
    THROW_MBSIMERROR("(NodeBasedBody::updateJacobians): Not implemented.");
  }

  void NodeBasedBody::updateGyroscopicAccelerations(NodeFrame* frame) {
    THROW_MBSIMERROR("(NodeBasedBody::updateGyroscopicAccelerations): Not implemented.");
  }

  void NodeBasedBody::addFrame(NodeFrame *frame) {
    Body::addFrame(frame);
  }

}
