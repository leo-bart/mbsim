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
#include "mbsim/contours/planar_contour.h"
#include "mbsim/functions/function.h"
#include "mbsim/utils/contact_utils.h"

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, PlanarContour)

  PlanarContour::~PlanarContour() {
     if (funcCrPC) 
       delete funcCrPC;
     funcCrPC=NULL;
  }

  Vec3 PlanarContour::evalKrPS(const Vec2 &zeta) {
    return (*funcCrPC)(zeta(0));
  }

  Vec3 PlanarContour::evalKs(const Vec2 &zeta) {
    return funcCrPC->parDer(zeta(0));
  }

  Vec3 PlanarContour::evalKt(const Vec2 &zeta) {
    static Vec3 Kt("[0;0;1]");
    return Kt;
  }

  Vec3 PlanarContour::evalParDer1Ks(const Vec2 &zeta) {
    return funcCrPC->parDerParDer(zeta(0));
  }

  void PlanarContour::init(InitStage stage) {
    if (stage == preInit) {
      if (etaNodes.size() < 2)
        THROW_MBSIMERROR("(PlanarContour::init): Size of etaNodes must be greater than 1.");
    }
    else if(stage==plotting) {
      if(plotFeature[openMBV]==enabled && openMBVRigidBody) {
        shared_ptr<vector<shared_ptr<OpenMBV::PolygonPoint> > > vpp = make_shared<vector<shared_ptr<OpenMBV::PolygonPoint> > >();
        if(not(ombvNodes.size())) {
          ombvNodes.resize(101);
          for(int i=0; i<101; i++)
            ombvNodes[i] = etaNodes[0] + (etaNodes[etaNodes.size()-1]-etaNodes[0])*i/100.;
        }
        for (unsigned int i=0; i<ombvNodes.size(); i++) {
          const Vec3 CrPC=(*funcCrPC)(ombvNodes[i]);
          vpp->push_back(OpenMBV::PolygonPoint::create(CrPC(0), CrPC(1), 0));
        }
        static_pointer_cast<OpenMBV::Extrusion>(openMBVRigidBody)->setHeight(0);
        static_pointer_cast<OpenMBV::Extrusion>(openMBVRigidBody)->addContour(vpp);
      }
    }
    RigidContour::init(stage);

    funcCrPC->init(stage);
  }

  void PlanarContour::setContourFunction(Function<Vec3(double)> *func) {
    funcCrPC=func;
    funcCrPC->setParent(this);
    funcCrPC->setName("Contour");
  }

  double PlanarContour::getCurvature(const Vec2 &zeta) {
    const Vec3 rs = funcCrPC->parDer(zeta(0));
    return nrm2(crossProduct(rs,funcCrPC->parDerParDer(zeta(0))))/pow(nrm2(rs),3);
  }

  void PlanarContour::initializeUsingXML(DOMElement * element) {
    RigidContour::initializeUsingXML(element);
    DOMElement * e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"nodes");
    etaNodes=getVec(e);
    e=E(element)->getFirstElementChildNamed(MBSIM%"contourFunction");
    setContourFunction(ObjectFactory::createAndInit<Function<Vec3(double)> >(e->getFirstElementChild()));
    e=E(element)->getFirstElementChildNamed(MBSIM%"open");
    if(e) setOpen(Element::getBool(e));
    e=E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBV");
    if(e) {
      DOMElement *ee=E(e)->getFirstElementChildNamed(MBSIM%"nodes");
      if(ee) ombvNodes=getVec(ee);
      OpenMBVExtrusion ombv;
      openMBVRigidBody=ombv.createOpenMBV(e); 
    }
  }

}
