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

#ifndef _OBSERVER__H_
#define _OBSERVER__H_

#include "element.h"

namespace MBSimGUI {

  class Observer : public Element {
    public:
      Observer(const QString &str="") : Element(str) { }
      QMenu* createContextMenu() { return new ObserverContextMenu(this); }
  };

  class KinematicCoordinatesObserver : public Observer {
    public:
      KinematicCoordinatesObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "KinematicCoordinatesObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new KinematicCoordinatesObserverPropertyDialog(this);}
  };

  class RelativeKinematicsObserver : public Observer {
    public:
      RelativeKinematicsObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "RelativeKinematicsObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new RelativeKinematicsObserverPropertyDialog(this);}
  };

  class MechanicalLinkObserver : public Observer {
    public:
      MechanicalLinkObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "MechanicalLinkObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new MechanicalLinkObserverPropertyDialog(this);}
  };

  class MechanicalConstraintObserver : public Observer {
    public:
      MechanicalConstraintObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "MechanicalConstraintObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new MechanicalConstraintObserverPropertyDialog(this);}
  };

  class ContactObserver : public Observer {
    public:
      ContactObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "ContactObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new ContactObserverPropertyDialog(this);}
  };

  class FrameObserver : public Observer {
    public:
      FrameObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "FrameObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new FrameObserverPropertyDialog(this);}
  };

  class RigidBodyObserver : public Observer {
    public:
      RigidBodyObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "RigidBodyObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new RigidBodyObserverPropertyDialog(this);}
  };

  class RigidBodySystemObserver : public Observer {
    public:
      RigidBodySystemObserver(const QString &str="") : Observer(str) { }
      QString getType() const { return "RigidBodySystemObserver"; }
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      ElementPropertyDialog* createPropertyDialog() {return new RigidBodySystemObserverPropertyDialog(this);}
  };
}

#endif
