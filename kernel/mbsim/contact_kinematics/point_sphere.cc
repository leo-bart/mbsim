/* Copyright (C) 2004-2013 MBSim Development Team
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
#include "point_sphere.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsim/contours/point.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/utils/eps.h"
#include "mbsim/utils/contact_utils.h"

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsPointSphere::assignContours(const vector<Contour*> &contour) {
    if(dynamic_cast<Point*>(contour[0])) {
      ipoint = 0;
      isphere = 1;
      point = static_cast<Point*>(contour[0]);
      sphere = static_cast<Sphere*>(contour[1]);
    }
    else {
      ipoint = 1;
      isphere = 0;
      point = static_cast<Point*>(contour[1]);
      sphere = static_cast<Sphere*>(contour[0]);
    }
  }

  void ContactKinematicsPointSphere::updateg(double &g, std::vector<ContourFrame*> &cFrame, int index) {
    Vec3 Wd = sphere->getFrame()->evalPosition() - point->getFrame()->evalPosition();
    double l = nrm2(Wd);
    Wd = Wd/l;
    g = l-sphere->getRadius();
    Vec3 t_;
    if(fabs(Wd(0))<epsroot() && fabs(Wd(1))<epsroot()) {
      t_(0) = 1.;
      t_(1) = 0.;
      t_(2) = 0.;
    }
    else {
      t_(0) = -Wd(1);
      t_(1) = Wd(0);
      t_(2) = 0.0;
    }
    t_ = t_/nrm2(t_);
    cFrame[ipoint]->getOrientation(false).set(0, Wd);
    cFrame[isphere]->getOrientation(false).set(0, -cFrame[ipoint]->getOrientation(false).col(0));
    cFrame[ipoint]->getOrientation(false).set(1, t_);
    cFrame[isphere]->getOrientation(false).set(1, -cFrame[ipoint]->getOrientation(false).col(1));
    cFrame[ipoint]->getOrientation(false).set(2, crossProduct(Wd,t_));
    cFrame[isphere]->getOrientation(false).set(2, cFrame[ipoint]->getOrientation(false).col(2));
    cFrame[ipoint]->setPosition(point->getFrame()->getPosition());
    cFrame[isphere]->setPosition(sphere->getFrame()->getPosition() - sphere->getRadius() * Wd);
  }

  void ContactKinematicsPointSphere::updatewb(Vec &wb, double g, vector<ContourFrame*> &cFrame) {
    const Vec3 n1 = cFrame[ipoint]->evalOrientation().col(0);
    const Vec3 u1 = cFrame[ipoint]->getOrientation().col(1);
    const Vec3 v1 = cFrame[ipoint]->getOrientation().col(2);
    Vec2 zeta1 = computeAnglesOnUnitSphere(point->getFrame()->evalOrientation().T()*n1);
    const Mat3x2 U1 = point->evalWU(zeta1);
    const Mat3x2 V1 = point->evalWV(zeta1);
    const Mat3x2 N1 = point->evalWN(zeta1);

    const Vec3 n2 = cFrame[isphere]->evalOrientation().col(0);
    const Vec3 u2 = cFrame[isphere]->evalOrientation().col(1);
    const Vec3 v2 = cFrame[isphere]->getOrientation().col(2);
    Vec2 zeta2 = computeAnglesOnUnitSphere(sphere->getFrame()->evalOrientation().T()*n2);
    const Mat3x2 R2 = sphere->evalWR(zeta2);
    const Mat3x2 U2 = sphere->evalWU(zeta2);
    const Mat3x2 V2 = sphere->evalWV(zeta2);

    const Vec3 vC1 = cFrame[ipoint]->evalVelocity();
    const Vec3 vC2 = cFrame[isphere]->evalVelocity();
    const Vec3 Om1 = cFrame[ipoint]->evalAngularVelocity();
    const Vec3 Om2 = cFrame[isphere]->evalAngularVelocity();

    SqrMat A(4,NONINIT);
    A(RangeV(0,0),RangeV(0,1)).init(0);// = -u1.T()*R1;
    A(RangeV(0,0),RangeV(2,3)) = u1.T()*R2;
    A(RangeV(1,1),RangeV(0,1)).init(0);// = -v1.T()*R1;
    A(RangeV(1,1),RangeV(2,3)) = v1.T()*R2;
    A(RangeV(2,2),RangeV(0,1)) = u2.T()*N1;
    A(RangeV(2,2),RangeV(2,3)) = n1.T()*U2;
    A(RangeV(3,3),RangeV(0,1)) = v2.T()*N1;
    A(RangeV(3,3),RangeV(2,3)) = n1.T()*V2;

    Vec b(4,NONINIT);
    b(0) = -u1.T()*(vC2-vC1);
    b(1) = -v1.T()*(vC2-vC1);
    b(2) = -v2.T()*(Om2-Om1);
    b(3) = u2.T()*(Om2-Om1);
    Vec zetad =  slvLU(A,b);
    Vec zetad1 = zetad(0,1);
    Vec zetad2 = zetad(2,3);

    const Mat3x3 tOm1 = tilde(Om1);
    const Mat3x3 tOm2 = tilde(Om2);

    wb(0) += ((vC2-vC1).T()*N1/**-n1.T()*tOm1*R1**/)*zetad1+n1.T()*(tOm2*R2*zetad2-tOm1*(vC2-vC1));
    if (wb.size()>1) {
      wb(1) += ((vC2-vC1).T()*U1/**-u1.T()*tOm1*R1**/)*zetad1+u1.T()*(tOm2*R2*zetad2-tOm1*(vC2-vC1));
      if (wb.size()>2)
        wb(2) += ((vC2-vC1).T()*V1/**-v1.T()*tOm1*R1**/)*zetad1+v1.T()*(tOm2*R2*zetad2-tOm1*(vC2-vC1));
    }
  }

}
