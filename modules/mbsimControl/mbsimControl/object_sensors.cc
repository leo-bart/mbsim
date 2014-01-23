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
#include "mbsimControl/object_sensors.h"
#include "mbsimControl/defines.h"
#include "mbsim/object.h"
#include "mbsim/dynamic_system.h"

using namespace fmatvec;
using namespace MBXMLUtils;
using namespace MBSim;

namespace MBSimControl {

  void GeneralizedCoordinateSensor::initializeUsingXML(TiXmlElement *element) {
    Sensor::initializeUsingXML(element);
    TiXmlElement *e=element->FirstChildElement(MBSIMCONTROLNS"object");
    objectString=e->Attribute("ref");
    e=element->FirstChildElement(MBSIMCONTROLNS"index");
    index=getInt(e);
  }

  void GeneralizedCoordinateSensor::init(InitStage stage) {
    if (stage==MBSim::resolveXMLPath) {
      if (objectString!="")
        setObject(getByPath<Object>(objectString));
      Sensor::init(stage);
    }
    else
      Sensor::init(stage);
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Element, GeneralizedPositionSensor, MBSIMCONTROLNS"GeneralizedPositionSensor")

  Vec GeneralizedPositionSensor::getSignal() {
    if (object->getq().size()==0)
      //return object->getq0()(fmatvec::Index(index, index));
      return fmatvec::Vec(1, fmatvec::INIT, 0);
    else
      return ((object->getq()).copy())(fmatvec::Index(index, index));
  }

  MBSIM_OBJECTFACTORY_REGISTERXMLNAME(Element, GeneralizedVelocitySensor, MBSIMCONTROLNS"GeneralizedVelocitySensor")

  Vec GeneralizedVelocitySensor::getSignal() {
    if (object->getu().size()==0)
      //return object->getu0()(fmatvec::Index(index, index));
      return fmatvec::Vec(1, fmatvec::INIT, 0);
    else
      return ((object->getu()).copy())(fmatvec::Index(index, index));
  }

}