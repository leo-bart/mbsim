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
 * Contact: thschindler@users.berlios.de
 */

#include<config.h>
#include "mbsim/contours/plane.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/grid.h>
#endif

using namespace std;
using namespace fmatvec;

namespace MBSim {

  Vec2 Plane::computeLagrangeParameter(const fmatvec::Vec3 &WrPoint) {
    cout << "WARNING: Plane::computeLagrangeParameter() just implemented for convenience till now. Proof correctness first before use." << endl;
    cout << "The parameters are the y and z coordinates : " << (R.getOrientation().T() *(WrPoint - R.getPosition()) )(Range<Fixed<1,2> >()) << endl;
    return (R.getOrientation().T() *(WrPoint - R.getPosition()) )(Range<Fixed<1,2> >()); //TODO: proof correctness (till now just implemented for convenience)
  }

#ifdef HAVE_OPENMBVCPPINTERFACE
  void Plane::enableOpenMBV(bool enable, double size, int number) {
    if(enable) {
      cout << "WARNING: Plane::enableOpenMBV() --> It might not work relieable ..." << endl;
      openMBVRigidBody=new OpenMBV::Grid;
      ((OpenMBV::Grid*)openMBVRigidBody)->setXSize(size);
      ((OpenMBV::Grid*)openMBVRigidBody)->setYSize(size);
      ((OpenMBV::Grid*)openMBVRigidBody)->setXNumber(number);
      ((OpenMBV::Grid*)openMBVRigidBody)->setYNumber(number);
      ((OpenMBV::Grid*)openMBVRigidBody)->setInitialRotation(0.,M_PI/2.,0.);
    }
    else openMBVRigidBody=0;
  }
#endif

  void Plane::initializeUsingXML(TiXmlElement *element) {
    RigidContour::initializeUsingXML(element);
#ifdef HAVE_OPENMBVCPPINTERFACE
    TiXmlElement * e=element->FirstChildElement(MBSIMNS"enableOpenMBV");
    if (e) {
      TiXmlElement* ee;
      ee=e->FirstChildElement(MBSIMNS"size");
      double size=getDouble(ee);
      ee=e->FirstChildElement(MBSIMNS"numberOfLines");
      int n=getInt(ee);
      enableOpenMBV(true, size, n);
    }
#endif
  }

}

