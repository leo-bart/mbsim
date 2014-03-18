/* Copyright (C) 2004-2009 MBSim Development Team
 * 
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 * 
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details. 
 *
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: friedrich.at.gc@googlemail.com
 */

#include <config.h>
#include "mbsim/environment.h"
#include "mbsim/element.h"
#include "mbsim/utils/utils.h"

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;

namespace MBSim {

  MBSimEnvironment *MBSimEnvironment::instance=NULL;

  MBSIM_OBJECTFACTORY_REGISTERXMLNAMEASSINGLETON(Environment, MBSimEnvironment, MBSIMNS"MBSimEnvironment")

  void MBSimEnvironment::initializeUsingXML(TiXmlElement *element) {
    Environment::initializeUsingXML(element);
    TiXmlElement *e;
    e=element->FirstChildElement(MBSIMNS"accelerationOfGravity");
    setAccelerationOfGravity(Element::getVec3(e));
  }

  TiXmlElement* MBSimEnvironment::writeXMLFile(TiXmlNode *parent) {
    TiXmlElement* ele0 = new TiXmlElement( MBSIMNS"MBSimEnvironment" );
    addElementText(ele0,MBSIMNS"accelerationOfGravity",getAccelerationOfGravity());
    parent->LinkEndChild(ele0);
    return ele0;
  }

}
