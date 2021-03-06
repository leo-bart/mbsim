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

#include <config.h> 
#include "sphere_sphere.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsim/contours/sphere.h"
#include "mbsim/utils/eps.h"

using namespace fmatvec;
using namespace std;

namespace MBSim {

  void ContactKinematicsSphereSphere::assignContours(const vector<Contour*> &contour) {
    isphere0 = 0; isphere1 = 1;
    sphere0 = static_cast<Sphere*>(contour[0]);
    sphere1 = static_cast<Sphere*>(contour[1]);
  }

  void ContactKinematicsSphereSphere::updateg(double &g, std::vector<ContourFrame*> &cFrame, int index) {
    Vec3 Wd = sphere1->getFrame()->evalPosition() - sphere0->getFrame()->evalPosition();
    double l = nrm2(Wd);
    Wd = Wd/l;
    g = l-sphere0->getRadius()-sphere1->getRadius();
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
    cFrame[isphere0]->getOrientation(false).set(0, Wd);
    cFrame[isphere1]->getOrientation(false).set(0, -cFrame[isphere0]->getOrientation(false).col(0));
    cFrame[isphere0]->getOrientation(false).set(1, t_);
    cFrame[isphere1]->getOrientation(false).set(1, -cFrame[isphere0]->getOrientation(false).col(1));
    cFrame[isphere0]->getOrientation(false).set(2, crossProduct(Wd,t_));
    cFrame[isphere1]->getOrientation(false).set(2, cFrame[isphere0]->getOrientation(false).col(2));
    cFrame[isphere0]->setPosition(sphere0->getFrame()->getPosition() + sphere0->getRadius() * Wd);
    cFrame[isphere1]->setPosition(sphere1->getFrame()->getPosition() - sphere1->getRadius() * Wd);
  }

}


//  void ContactKinematicsSphereSphere::stage2(const Vec& g, Vec &gd, vector<ContourPointData> &cpData) {

//    Vec WrPC[2], WvC[2];
//
//    WrPC[isphere1] = cpData[isphere0].Wn*sphere1->getRadius();
//    cpData[isphere1].WrOC = sphere1->getWrOP()+WrPC[isphere1];
//    WrPC[isphere0] = cpData[isphere0].Wn*(-sphere0->getRadius());
//    cpData[isphere0].WrOC = sphere0->getWrOP()+WrPC[isphere0];
//    WvC[isphere0] = sphere0->evalWvP()+crossProduct(sphere0->getWomegaC(),WrPC[isphere0]);
//    WvC[isphere1] = sphere1->evalWvP()+crossProduct(sphere1->getWomegaC(),WrPC[isphere1]);
//    Vec WvD = WvC[isphere0] - WvC[isphere1];
//    gd(0) = trans(cpData[isphere0].Wn)*WvD;
//
//    if(cpData[isphere0].Wt.cols()) {
//      cpData[isphere0].Wt.col(0) = computeTangential(cpData[isphere0].Wn);
//      if(cpData[isphere0].Wt.cols()==2) {
//        cpData[isphere0].Wt.col(1) = crossProduct(cpData[isphere0].Wn ,cpData[isphere0].Wt.col(0));
//        cpData[isphere1].Wt  = -cpData[isphere0].Wt ; 
//        static RangeV iT(1,cpData[isphere0].Wt.cols());
//        gd(iT) = trans(cpData[isphere0].Wt)*WvD;
//      }
//    }
//  }
