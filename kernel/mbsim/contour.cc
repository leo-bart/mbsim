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
 *          rzander@users.berlios.de
 */

#include <config.h>
#include "mbsim/contour.h"
#include "mbsim/contours/line.h"
#include "mbsim/contours/point.h"
#include "mbsim/utils/utils.h"
#include "mbsim/object.h"
#include "mbsim/mbsim_event.h"
#include "mbsim/utils/rotarymatrices.h"
#include "mbsim/contours/point.h"
#include "mbsim/utils/contact_utils.h"

#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/group.h>
#include <openmbvcppinterface/rigidbody.h>
#include <openmbvcppinterface/frustum.h>
#include <openmbvcppinterface/sphere.h>
#endif

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  /* Contour */
  Contour::Contour(const string &name, Frame *R_) : Element(name), R(R_) {
    // no canonic output...
    hSize[0] = 0;
    hSize[1] = 0;
    hInd[0] = 0;
    hInd[1] = 0;
  }

  Contour::~Contour() {}

  void Contour::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(saved_frameOfReference!="")
        setFrameOfReference(getByPath<Frame>(saved_frameOfReference));
      Element::init(stage);
    }
    else if(stage==unknownStage) {
//      getFrame()->getJacobianOfTranslation(0).resize(hSize[0]);
//      getFrame()->getJacobianOfRotation(0).resize(hSize[0]);
//      getFrame()->getJacobianOfTranslation(1).resize(hSize[1]);
//      getFrame()->getJacobianOfRotation(1).resize(hSize[1]);
    }
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
	//
//   plotGroup=new H5::Group(parent->getPlotGroup()->createGroup(name));
//          H5::SimpleAttribute<string>::setData(*plotGroup, "Description", "Object of class: "+getType());
//
//          plotColumns.insert(plotColumns.begin(), "Time");
//          if(plotColumns.size()>1) {
//            plotVectorSerie=new H5::VectorSerie<double>;
//            // copy plotColumns to a std::vector
//            vector<string> dummy; copy(plotColumns.begin(), plotColumns.end(), insert_iterator<vector<string> >(dummy, dummy.begin()));
//            plotVectorSerie->create(*plotGroup,"data",dummy);
//            plotVectorSerie->setDescription("Default dataset for class: "+getType());
//          }
//          plotVector.clear();
//          plotVector.reserve(plotColumns.size()); // preallocation
//
	// 
	Element::init(stage);
        //R.init(stage);
	//if(plotGroup) {
	//H5::Group *plotGroup = new H5::Group(getPlotGroup()->createGroup(R.getName()));
	// H5::SimpleAttribute<string>::setData(*plotGroup, "Description", "Object of class: "+getType());
	// plotColumns.insert(plotColumns.begin(), "Time");
	// if(plotColumns.size()>1) {
	//   plotVectorSerie=new H5::VectorSerie<double>;
	//   // copy plotColumns to a std::vector
	//   vector<string> dummy; copy(plotColumns.begin(), plotColumns.end(), insert_iterator<vector<string> >(dummy, dummy.begin()));
	//   plotVectorSerie->create(*plotGroup,"data",dummy);
	//   plotVectorSerie->setDescription("Default dataset for class: "+getType());
	// }
	// plotVector.clear();
	// plotVector.reserve(plotColumns.size()); // preallocation
        //}
      }
    }
    else
      Element::init(stage);
  }

  Frame* Contour::createContourFrame(const string &name) const {
    return new Frame(name);
  }

  void Contour::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
      Element::plot(t,dt);
      //R.plot(t,dt);
      //if(getPlotFeature(plotRecursive)==enabled) {
      //  if(plotColumns.size()>1) {
      //    plotVector.insert(plotVector.begin(), t);
      //    assert(plotColumns.size()==plotVector.size());
      //    plotVectorSerie->append(plotVector);
      //    plotVector.clear();
      //  }
      //}
    }
  }

  Vec3 Contour::getPosition(double t, const fmatvec::Vec2 &zeta) {
    return R->getPosition(t) + getWrPS(t,zeta);
  }

  Vec3 Contour::getWu(double t, const fmatvec::Vec2 &zeta) {
    Vec3 Ws=getWs(t,zeta);
    return Ws/nrm2(Ws);
  }

  Vec3 Contour::getWv(double t, const fmatvec::Vec2 &zeta) {
    Vec3 Wt=getWt(t,zeta);
    return Wt/nrm2(Wt);
  }

  Vec3 Contour::getWn(double t, const fmatvec::Vec2 &zeta) {
    Vec3 N=crossProduct(getWs(t,zeta),getWt(t,zeta));
    return N/nrm2(N);
  }

  Vec3 Contour::getParDer1Ku(const fmatvec::Vec2 &zeta) {
    Vec3 Ks = getKs(zeta);
    Vec3 parDer1Ks = getParDer1Ks(zeta);
    return parDer1Ks/nrm2(Ks) - Ks*((Ks.T()*parDer1Ks)/pow(nrm2(Ks),3));
  }

  Vec3 Contour::getParDer2Ku(const fmatvec::Vec2 &zeta) {
    THROW_MBSIMERROR("(Contour::getParDer2Ku): Not implemented.");
    return 0;
  }

  Vec3 Contour::getParDer1Kv(const fmatvec::Vec2 &zeta) {
    THROW_MBSIMERROR("(Contour::getParDer1Kv): Not implemented.");
    return 0;
  }

  Vec3 Contour::getParDer2Kv(const fmatvec::Vec2 &zeta) {
    THROW_MBSIMERROR("(Contour::getParDer2Kv): Not implemented.");
    return 0;
  }

  Vec3 Contour::getParDer1Wn(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer1Kn(zeta);
  }

  Vec3 Contour::getParDer2Wn(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer2Kn(zeta);
  }

  Vec3 Contour::getParDer1Wu(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer1Ku(zeta);
  }

  Vec3 Contour::getParDer2Wu(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer2Ku(zeta);
  }

  Vec3 Contour::getParDer1Wv(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer1Kv(zeta);
  }

  Vec3 Contour::getParDer2Wv(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getParDer2Kv(zeta);
  }

  Vec3 Contour::getWrPS(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getKrPS(zeta);
  }

  Vec3 Contour::getWs(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getKs(zeta);
  }

  Vec3 Contour::getWt(double t, const fmatvec::Vec2 &zeta) {
    return R->getOrientation(t)*getKt(zeta);
  }

  Mat3x2 Contour::getWN(double t, const fmatvec::Vec2 &zeta) {
    Mat3x2 WN(NONINIT);
    WN.set(0,getParDer1Wn(t,zeta));
    WN.set(1,getParDer2Wn(t,zeta));
    return WN;
  }

  Mat3x2 Contour::getWR(double t, const fmatvec::Vec2 &zeta) {
    Mat3x2 WR(NONINIT);
    WR.set(0,getWs(t,zeta));
    WR.set(1,getWt(t,zeta));
    return WR;
  }

  Mat3x2 Contour::getWU(double t, const fmatvec::Vec2 &zeta) {
    Mat3x2 WU(NONINIT);
    WU.set(0,getParDer1Wu(t,zeta));
    WU.set(1,getParDer2Wu(t,zeta));
    return WU;
  }

  Mat3x2 Contour::getWV(double t, const fmatvec::Vec2 &zeta) {
    Mat3x2 WV(NONINIT);
    WV.set(0,getParDer1Wv(t,zeta));
    WV.set(1,getParDer2Wv(t,zeta));
    return WV;
  }

  void Contour::initializeUsingXML(DOMElement *element) {
    Element::initializeUsingXML(element);
    DOMElement *ec=E(element)->getFirstElementChildNamed(MBSIM%"frameOfReference");
    if(ec) setFrameOfReference(E(ec)->getAttribute("ref"));
  }

  DOMElement* Contour::writeXMLFile(DOMNode *parent) {
    DOMElement *ele0 = Element::writeXMLFile(parent);
  //   if(getFrameOfReference()) {
  //      DOMElement *ele1 = new DOMElement( MBSIM%"frameOfReference" );
  //      string str = string("../Frame[") + getFrameOfReference()->getName() + "]";
  //      ele1->SetAttribute("ref", str);
  //      ele0->LinkEndChild(ele1);
  //    }
   return ele0;
  }

  /* Rigid Contour */
  RigidContour::~RigidContour() {
  }

  void RigidContour::init(InitStage stage) {
    if(stage==unknownStage)
      Contour::init(stage);
    else if(stage==plotting) {
      updatePlotFeatures();

      if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
        if(getPlotFeature(openMBV)==enabled && openMBVRigidBody) {
          openMBVRigidBody->setName(name);
          parent->getOpenMBVGrp()->addObject(openMBVRigidBody);
        }
#endif
        Contour::init(stage);
      }
    }
    else
      Contour::init(stage);
  }
  
  Frame* RigidContour::createContourFrame(const string &name) const {
    FloatingRelativeFrame *frame = new FloatingRelativeFrame(name);      
    frame->setFrameOfReference(R);
    return frame;
  }

   void RigidContour::plot(double t, double dt) {
    if(getPlotFeature(plotRecursive)==enabled) {
#ifdef HAVE_OPENMBVCPPINTERFACE
      if(getPlotFeature(openMBV)==enabled && openMBVRigidBody) {
        vector<double> data;
        data.push_back(t);
        data.push_back(R->getPosition(t)(0));
        data.push_back(R->getPosition()(1));
        data.push_back(R->getPosition()(2));
        Vec3 cardan=AIK2Cardan(R->getOrientation(t));
        data.push_back(cardan(0));
        data.push_back(cardan(1));
        data.push_back(cardan(2));
        data.push_back(0);
        openMBVRigidBody->append(data);
      }
#endif
      Contour::plot(t,dt);
    }
  }

  ContactKinematics * RigidContour::findContactPairingWith(std::string type0, std::string type1) {
    return findContactPairingRigidRigid(type0.c_str(), type1.c_str());
  }

}

