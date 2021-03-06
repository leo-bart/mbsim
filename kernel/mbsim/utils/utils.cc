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
#include "mbsim/utils/utils.h"
#include "mbsim/utils/eps.h"

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;

namespace MBSim {

  std::string numtostr(int i) {
    std::ostringstream oss;
    oss << i;
    return oss.str();
  }

  std::string numtostr(double d) {
    std::ostringstream oss;
    oss << d;
    return oss.str();
  }

  double sign(double x) {
    if (x > 0.)
      return 1.;
    else if (x < 0.)
      return -1.;
    else
      return 0.;
  }

  double ArcTan(double x, double y) {
    double phi;
    phi = atan2(y, x);

    if (phi < -MBSim::macheps())
      phi += 2 * M_PI;
    return phi;
  }

  Mat cs2Mat(cs* sparseMat) {
    Mat newMat(sparseMat->m, sparseMat->n, INIT, 0.);

    if (sparseMat->nz < 0) {
      for (int j = 0; j < sparseMat->n; j++) {
        for (int p = sparseMat->p[j]; p < sparseMat->p[j + 1]; p++) {
          int row = sparseMat->i[p];
          int col = j;
          double entry = sparseMat->x ? sparseMat->x[p] : 1;
          newMat(row, col) = entry;
        }
      }
    }
    else {
      for (int i = 0; i < sparseMat->nz; i++) {
        int row = sparseMat->i[i];
        int col = sparseMat->p[i];
        double entry = sparseMat->x ? sparseMat->x[i] : 1;
        newMat(row, col) = entry;
      }
    }

    return newMat;
  }

  Vec3 computeTangential(const Vec3 &n) {
    Vec3 t(NONINIT);
    if(fabs(n(0))+fabs(n(1)) > 1e-12) {
      t(2)=0;
      double buf = pow(n(0),2)+pow(n(1),2);
      buf = 1.0/sqrt(buf);
      t(0) = n(1)*buf;
      t(1) = -n(0)*buf;
    }
    else {
      t(0)=0;
      double buf = pow(n(1),2)+pow(n(2),2);
      buf = 1.0/sqrt(buf);
      t(1) = n(2)*buf;
      t(2) = -n(1)*buf;
    }
    return t;
  }

}
