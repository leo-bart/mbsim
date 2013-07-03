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
 * Contact: markus.ms.schneider@gmail.com
 */

#include <config.h>
#include "mbsimHydraulics/objectfactory.h"
#include "mbsimHydraulics/rigid_line.h"
#include "mbsimHydraulics/pressure_loss.h"
#include "mbsimHydraulics/hnode_mec.h"
#include "mbsimHydraulics/environment.h"
#include "mbsimHydraulics/controlvalve43.h"
#include "mbsimHydraulics/checkvalve.h"
#include "mbsimHydraulics/leakage_line.h"
#include "mbsimHydraulics/dimensionless_line.h"
#include "mbsimHydraulics/hydraulic_sensor.h"
#include "mbsimHydraulics/elastic_line_galerkin.h"
#include "mbsimHydraulics/elastic_line_variational.h"

using namespace std;
using namespace MBXMLUtils;
using namespace MBSim;

namespace MBSimHydraulics {

  ObjectFactory *ObjectFactory::instance=NULL;


  void ObjectFactory::initialize() {
    if(instance==0) {
      instance=new ObjectFactory;
      MBSim::ObjectFactory::getInstance()->registerObjectFactory(instance);
    }
  }


  Function1<double, double> * ObjectFactory::createFunction1_SS(TiXmlElement * element) {
    if (element==0) return 0;
    if (element->ValueStr()==MBSIMHYDRAULICSNS"SerialResistanceLinePressureLoss")
      return new SerialResistanceLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ParallelResistanceLinePressureLoss")
      return new ParallelResistanceLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ZetaLinePressureLoss")
      return new ZetaLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ZetaPosNegLinePressureLoss")
      return new ZetaPosNegLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"LaminarTubeFlowLinePressureLoss")
      return new LaminarTubeFlowLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"TurbulentTubeFlowLinePressureLoss")
      return new TurbulentTubeFlowLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"CurveFittedLinePressureLoss")
      return new CurveFittedLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"TabularLinePressureLoss")
      return new TabularLinePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"RelativeAreaZetaClosablePressureLoss")
      return new RelativeAreaZetaClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"GapHeightClosablePressureLoss")
      return new GapHeightClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ReynoldsClosablePressureLoss")
      return new ReynoldsClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"RelativeAlphaClosablePressureLoss")
      return new RelativeAlphaClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"GammaCheckvalveClosablePressureLoss")
      return new GammaCheckvalveClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"IdelchickCheckvalveClosablePressureLoss")
      return new IdelchickCheckvalveClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ConeCheckvalveClosablePressureLoss")
      return new ConeCheckvalveClosablePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"PlaneLeakagePressureLoss")
      return new PlaneLeakagePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"EccentricCircularLeakagePressureLoss")
      return new EccentricCircularLeakagePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"RealCircularLeakagePressureLoss")
      return new RealCircularLeakagePressureLoss();
    if (element->ValueStr()==MBSIMHYDRAULICSNS"UnidirectionalZetaPressureLoss")
      return new UnidirectionalZetaPressureLoss();
    return 0;
  }


  Object * ObjectFactory::createObject(TiXmlElement * element) {
    if (element==0) return 0;
    if (element->ValueStr()==MBSIMHYDRAULICSNS"RigidLine")
      return new RigidLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ClosableRigidLine")
      return new ClosableRigidLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"UnidirectionalRigidLine")
      return new UnidirectionalRigidLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"PlaneLeakageLine")
      return new PlaneLeakageLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"CircularLeakageLine")
      return new CircularLeakageLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"PlaneLeakage0DOF")
      return new PlaneLeakage0DOF(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"CircularLeakage0DOF")
      return new CircularLeakage0DOF(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ConstrainedLine")
      return new ConstrainedLine(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"FluidPump")
      return new FluidPump(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"StatelessOrifice")
      return new StatelessOrifice(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ElasticLineGalerkin")
      return new ElasticLineGalerkin(element->Attribute("name"));
    if (element->ValueStr()==MBSIMHYDRAULICSNS"ElasticLineVariational")
      return new ElasticLineVariational(element->Attribute("name"));
    return 0;
  }


  Link* ObjectFactory::createLink(TiXmlElement *element) {
    if(element==0) return 0;
    if(element->ValueStr()==MBSIMHYDRAULICSNS"ConstrainedNode")
      return new ConstrainedNode(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"EnvironmentNode")
      return new EnvironmentNode(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"ElasticNode")
      return new ElasticNode(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"RigidNode")
      return new RigidNode(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"RigidCavitationNode")
      return new RigidCavitationNode(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"PressurePump")
      return new PressurePump(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"ConstrainedNodeMec")
      return new ConstrainedNodeMec(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"EnvironmentNodeMec")
      return new EnvironmentNodeMec(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"ElasticNodeMec")
      return new ElasticNodeMec(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"RigidNodeMec")
      return new RigidNodeMec(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"FlowSensor")
      return new FlowSensor(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"PressureSensor")
      return new PressureSensor(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"TemperatureSensor")
      return new TemperatureSensor(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"KinematicViscositySensor")
      return new KinematicViscositySensor(element->Attribute("name"));
    return 0;
  }


  Environment* ObjectFactory::getEnvironment(TiXmlElement *element) {
    if(element==0) return 0;
    if(element->ValueStr()==MBSIMHYDRAULICSNS"HydraulicEnvironment")
      return HydraulicEnvironment::getInstance();
    return 0;
  }


  Group * ObjectFactory::createGroup(TiXmlElement * element) {
    if(element==0) return 0;
    if(element->ValueStr()==MBSIMHYDRAULICSNS"Controlvalve43")
      return new Controlvalve43(element->Attribute("name"));
    if(element->ValueStr()==MBSIMHYDRAULICSNS"Checkvalve")
      return new Checkvalve(element->Attribute("name"));
    return 0;
  }

  MBSim::ObjectFactoryBase::MM_PRINSPRE& ObjectFactory::getPriorityNamespacePrefix() {
    static MBSim::ObjectFactoryBase::MM_PRINSPRE priorityNamespacePrefix;

    if(priorityNamespacePrefix.empty()) {
      priorityNamespacePrefix.insert(P_PRINSPRE( 50, P_NSPRE(MBSIMHYDRAULICSNS_, "")));
      priorityNamespacePrefix.insert(P_PRINSPRE( 40, P_NSPRE(MBSIMHYDRAULICSNS_, "hyd")));
      priorityNamespacePrefix.insert(P_PRINSPRE( 30, P_NSPRE(MBSIMHYDRAULICSNS_, "hydraulic")));
      priorityNamespacePrefix.insert(P_PRINSPRE( 20, P_NSPRE(MBSIMHYDRAULICSNS_, "mbsimhydraulic")));
    }

    return priorityNamespacePrefix;
  }

}