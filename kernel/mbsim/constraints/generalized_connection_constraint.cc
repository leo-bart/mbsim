/* Copyright (C) 2004-2016 MBSim Development Team
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
#include "mbsim/constraints/generalized_connection_constraint.h"
#include "mbsim/links/generalized_connection.h"
#include <mbsim/constitutive_laws/bilateral_constraint.h>
#include "mbsim/objects/rigid_body.h"
#include "mbsim/dynamic_system.h"
#include "mbsim/objectfactory.h"
#ifdef HAVE_OPENMBVCPPINTERFACE
#include <openmbvcppinterface/arrow.h>
#include <openmbvcppinterface/frame.h>
#endif

using namespace std;
using namespace fmatvec;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSim {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIM, GeneralizedConnectionConstraint)

  GeneralizedConnectionConstraint::GeneralizedConnectionConstraint(const std::string &name) : GeneralizedConstraint(name), bi(NULL), bd(NULL) {
  }

  void GeneralizedConnectionConstraint::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if (saved_IndependentBody!="")
        setIndependentRigidBody(getByPath<RigidBody>(saved_IndependentBody));
      if (saved_DependentBody!="")
        setDependentRigidBody(getByPath<RigidBody>(saved_DependentBody));
      GeneralizedConstraint::init(stage);
    }
    else if(stage==preInit) {
      GeneralizedConstraint::init(stage);
      bd->addDependency(this);
      addDependency(bi);
    }
    else
      GeneralizedConstraint::init(stage);
  }

  void GeneralizedConnectionConstraint::updateGeneralizedCoordinates() {
    if(bi) {
      bd->getqRel(false) = bi->evalqRel();
      bd->getuRel(false) = bi->evaluRel();
    }
    updGC = false;
  }

  void GeneralizedConnectionConstraint::updateGeneralizedJacobians(int j) {
    if(bi)
      bd->getJRel(0,false)(Range<Var,Var>(0,bi->getuRelSize()-1),Range<Var,Var>(0,bi->gethSize()-1)) = bi->evalJRel();
    updGJ = false;
  }

  void GeneralizedConnectionConstraint::initializeUsingXML(DOMElement* element) {
    GeneralizedConstraint::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIM%"dependentRigidBody");
    saved_DependentBody=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIM%"independentRigidBody");
    if(e) saved_IndependentBody=E(e)->getAttribute("ref");

#ifdef HAVE_OPENMBVCPPINTERFACE
    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVForce");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toHead,OpenMBV::Arrow::toPoint,1,1);
      FArrow=ombv.createOpenMBV(e);
    }

    e = E(element)->getFirstElementChildNamed(MBSIM%"enableOpenMBVMoment");
    if (e) {
      OpenMBVArrow ombv("[-1;1;1]",0,OpenMBV::Arrow::toDoubleHead,OpenMBV::Arrow::toPoint,1,1);
      MArrow=ombv.createOpenMBV(e);
    }
#endif
  }

  void GeneralizedConnectionConstraint::setUpInverseKinetics() {
    GeneralizedConnection *connection = new GeneralizedConnection(string("GeneralizedConnection")+name);
    static_cast<DynamicSystem*>(parent)->addInverseKineticsLink(connection);
    connection->setRigidBodyFirstSide(bi);
    connection->setRigidBodySecondSide(bd);
    connection->setGeneralizedForceLaw(new BilateralConstraint);
    connection->setSupportFrame(support);
    if(FArrow)
      connection->setOpenMBVForce(FArrow);
    if(MArrow)
      connection->setOpenMBVMoment(MArrow);
  }

}
