/* Copyright (C) 2004-2014 MBSim Development Team
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
#include "floating_contour_frame.h"
#include "mbsim/contours/contour.h"

namespace MBSim {

  void FloatingContourFrame::updatePositions() {
    parent->updatePositions(this);
    updPos = false;
  }

 void FloatingContourFrame::updateVelocities() {
    contour->updateVelocities(this);
    updVel = false;
  }

  void FloatingContourFrame::updateAccelerations(double t) {
    contour->updateAccelerations(t,this);
    updAcc = true;
  }

  void FloatingContourFrame::updateJacobians(double t, int j) {
    contour->updateJacobians(t,this,j);
    updJac[j] = false;
  }

  void FloatingContourFrame::updateGyroscopicAccelerations(double t) {
    contour->updateGyroscopicAccelerations(t,this);
    updGA = false;
  }

}
