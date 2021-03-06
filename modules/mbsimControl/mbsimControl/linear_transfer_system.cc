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
 * Contact: martin.o.foerg@gmail.com
 */

#include <config.h>
#include <iostream>
#include "mbsimControl/linear_transfer_system.h"

using namespace std;
using namespace fmatvec;
using namespace MBSim;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimControl {

  MBSIM_OBJECTFACTORY_REGISTERCLASS(MBSIMCONTROL, LinearTransferSystem)

  void LinearTransferSystem::initializeUsingXML(DOMElement * element) {
    Signal::initializeUsingXML(element);
    DOMElement *e;
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"inputSignal");
    inputSignalString=E(e)->getAttribute("ref");
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"systemMatrix");
    if(e) A = Element::getSqrMat(e);
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"inputMatrix");
    if(e) B = Element::getMat(e, A.rows(), 0);
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"outputMatrix");
    if(e) C = Element::getMat(e, 0, A.cols());
    e=E(element)->getFirstElementChildNamed(MBSIMCONTROL%"feedthroughMatrix");
    if(e) D = Element::getSqrMat(e);
  }

  void LinearTransferSystem::updateSignal() {
    s = C*x + D*inputSignal->evalSignal();
    upds = false;
  }

  void LinearTransferSystem::updatexd() {
    xd = A*x + B*inputSignal->evalSignal();
  }

  void LinearTransferSystem::init(InitStage stage) {
    if(stage==resolveXMLPath) {
      if(inputSignalString!="")
        setInputSignal(getByPath<Signal>(inputSignalString));
      if(not inputSignal)
        THROW_MBSIMERROR("(LinearTransferSystem::init): input signal must be given");
    }
    else if(stage==preInit) {
      if(not A.size()) {
        B.resize(0,getSignalSize());
        C.resize(B.cols(),0);
      }
      else
      {
        if(B.rows() != A.size())
          THROW_MBSIMERROR("Number of rows of input matrix must be equal to size of state matrix");
        if(B.cols() != getSignalSize())
          THROW_MBSIMERROR("Number of columns of input matrix must be equal to signal size");
        if(C.rows() != B.cols())
          THROW_MBSIMERROR("Number of rows of output matrix must be equal to signal size");
        if(C.cols() != A.size())
          THROW_MBSIMERROR("Number of columns of output matrix must be equal size of state matrix");
      }
      if(not D.size())
        D.resize(getSignalSize());
      else {
        if(D.size() != C.rows())
          THROW_MBSIMERROR("Size of feedthrough matrix must be equal to signal size");
      }
    }
    Signal::init(stage);
  }

}
