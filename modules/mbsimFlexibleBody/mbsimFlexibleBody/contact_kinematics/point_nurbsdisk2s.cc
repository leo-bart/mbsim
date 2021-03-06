/* Copyright (C) 2004-2015 MBSim Development Team
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
 * Contact: thorsten.schindler@mytum.de
 */

#include <config.h>
#include "mbsimFlexibleBody/contact_kinematics/point_nurbsdisk2s.h"
#include "mbsimFlexibleBody/contours/nurbs_disk_2s.h"
#include "mbsim/frames/contour_frame.h"
#include "mbsim/contours/point.h"

using namespace fmatvec;
using namespace std;
using namespace MBSim;

namespace MBSimFlexibleBody {

  ContactKinematicsPointNurbsDisk2s::ContactKinematicsPointNurbsDisk2s() : ipoint(0), inurbsdisk(0), point(0), nurbsdisk(0) {
  }

  ContactKinematicsPointNurbsDisk2s::~ContactKinematicsPointNurbsDisk2s() {}

  void ContactKinematicsPointNurbsDisk2s::assignContours(const vector<Contour*> &contour) {
    if(dynamic_cast<Point*>(contour[0])) {
      ipoint = 0;
      inurbsdisk = 1;
      point = static_cast<Point*>(contour[0]);
      nurbsdisk = static_cast<NurbsDisk2s*>(contour[1]);
    }
    else {
      ipoint = 1;
      inurbsdisk = 0;
      point = static_cast<Point*>(contour[1]);
      nurbsdisk = static_cast<NurbsDisk2s*>(contour[0]);
    }
  }

  void ContactKinematicsPointNurbsDisk2s::updateg(double &g, vector<ContourFrame*> &cFrame, int index) {
    cFrame[ipoint]->setPosition(point->getFrame()->evalPosition()); // position of the point in worldcoordinates
    cFrame[inurbsdisk]->setZeta(nurbsdisk->transformCW(nurbsdisk->evalOrientation().T()*(cFrame[ipoint]->getPosition(false) - nurbsdisk->evalPosition()))(0,1)); // position of the point in the cylinder-coordinates of the disk -> NO CONTACTSEARCH

    if(nurbsdisk->isZetaOutside(cFrame[inurbsdisk]->getZeta()))
      g = 1.;
    else {
      cFrame[inurbsdisk]->setPosition(nurbsdisk->evalPosition(cFrame[inurbsdisk]->getZeta()));
      cFrame[inurbsdisk]->getOrientation(false).set(0, nurbsdisk->evalWn(cFrame[inurbsdisk]->getZeta()));
      cFrame[inurbsdisk]->getOrientation(false).set(1, nurbsdisk->evalWu(cFrame[inurbsdisk]->getZeta()));
      cFrame[inurbsdisk]->getOrientation(false).set(2, nurbsdisk->evalWv(cFrame[inurbsdisk]->getZeta()));

      cFrame[ipoint]->getOrientation(false).set(0, -cFrame[inurbsdisk]->getOrientation(false).col(0));
      cFrame[ipoint]->getOrientation(false).set(1, -cFrame[inurbsdisk]->getOrientation(false).col(1));
      cFrame[ipoint]->getOrientation(false).set(2,  cFrame[inurbsdisk]->getOrientation(false).col(2));   // to have a legal framework the second tangent is not the negative of the tanget of the disk

//      cout << "Normale: " <<  cFrame[inurbsdisk]->getOrientation(false).col(0) << endl;
//      cout << "1.Tangente: " <<  cFrame[inurbsdisk]->getOrientation(false).col(1) << endl;
//      cout << "2.Tangente: " <<  cFrame[inurbsdisk]->getOrientation(false).col(2) << endl;

      g = cFrame[inurbsdisk]->getOrientation(false).col(0).T() * (cFrame[ipoint]->getPosition(false) - cFrame[inurbsdisk]->getPosition(false));
      // cout << "Abstand: " << g << endl;
    }
  }

  void ContactKinematicsPointNurbsDisk2s::updatewb(Vec &wb, double g, vector<ContourFrame*> &cFrame) {
    throw MBSim::MBSimError("(ContactKinematicsPointNurbsDisk2s:updatewb): Not implemented!");
  }

}
