/*
    MBSimGUI - A fronted for MBSim.
    Copyright (C) 2012 Martin Förg

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef _DYNAMIC_SYSTEM_SOLVER__H_
#define _DYNAMIC_SYSTEM_SOLVER__H_

#include "group.h"
#include <string>

namespace MBSimGUI {

  class Environment : public QObject {
    public:
      static Environment *getInstance() { return instance?instance:(instance=new Environment); }

    protected:
      Environment() { }
      virtual ~Environment() { }
      static Environment *instance;
  };

  class DynamicSystemSolver : public Group {
    protected:
      xercesc::DOMElement *environments;
    public:
      DynamicSystemSolver(const QString &str="") : Group(str) { config = true; }
      QString getType() const { return "DynamicSystemSolver"; }
      xercesc::DOMElement* getXMLEnvironments() { return environments; }
      void removeXMLElements();
      xercesc::DOMElement* createXMLElement(xercesc::DOMNode *parent);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      QString getFileExtension() const { return ".mbsim.xml"; }

      ElementPropertyDialog* createPropertyDialog() { return new DynamicSystemSolverPropertyDialog(this); }
      QMenu* createContextMenu() { return new ElementContextMenu(this,NULL,false); }
  };

}

#endif
