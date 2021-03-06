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

#ifndef _GROUP__H_
#define _GROUP__H_

#include "element.h"

namespace MBSimGUI {

  class Frame;
  class Contour;
  class Object;
  class Link;
  class Observer;

  class Group : public Element {
    protected:
      std::vector<Frame*> frame;
      std::vector<Contour*> contour;
      std::vector<Group*> group;
      std::vector<Object*> object;
      std::vector<Link*> link;
      std::vector<Constraint*> constraint;
      std::vector<Observer*> observer;
      std::vector<Element*> removedElement;
      xercesc::DOMElement *frames, *contours, *groups, *objects, *links, *constraints, *observers;

    public:
      Group(const QString &str="");
      ~Group();
      QString getType() const { return "Group"; }
      void createXMLConstraints();
      void createXMLObservers();
      xercesc::DOMElement* getXMLFrames() { return frames; }
      xercesc::DOMElement* getXMLContours() { return contours; }
      xercesc::DOMElement* getXMLGroups() { return groups; }
      xercesc::DOMElement* getXMLObjects() { return objects; }
      xercesc::DOMElement* getXMLLinks() { return links; }
      xercesc::DOMElement* getXMLConstraints() { return constraints; }
      xercesc::DOMElement* getXMLObservers() { return observers; }
      xercesc::DOMElement* getXMLFrame() { return getXMLObservers()->getNextElementSibling(); }
      void removeXMLElements();
      xercesc::DOMElement* createXMLElement(xercesc::DOMNode *parent);
      xercesc::DOMElement* processFileID(xercesc::DOMElement* element);
      xercesc::DOMElement* processHref(xercesc::DOMElement* element);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      virtual Element *getChildByContainerAndName(const QString &container, const QString &name) const;
      void setActionPasteDisabled(bool flag);
      int getNumberOfFrames() {return frame.size();}
      int getNumberOfContours() {return contour.size();}
      int getNumberOfGroups() {return group.size();}
      int getNumberOfObjects() {return object.size();}
      int getNumberOfLinks() {return link.size();}
      int getNumberOfConstraints() {return constraint.size();}
      int getNumberOfObservers() {return observer.size();}
      int getIndexOfFrame(Frame *frame);
      int getIndexOfContour(Contour *contour);
      int getIndexOfGroup(Group *group);
      int getIndexOfObject(Object *object);
      int getIndexOfLink(Link *link);
      int getIndexOfConstraint(Constraint *constraint);
      int getIndexOfObserver(Observer *observer);
      Frame* getFrame(int i) const {return frame[i];}
      Contour* getContour(int i) const {return contour[i];}
      Object* getObject(int i) const {return object[i];}
      Group* getGroup(int i) const {return group[i];}
      Link* getLink(int i) const {return link[i];}
      Constraint* getConstraint(int i) const {return constraint[i];}
      Observer* getObserver(int i) const {return observer[i];}
      Frame* getFrame(const QString &name) const;
      Contour* getContour(const QString &name) const;
      Object* getObject(const QString &name) const;
      Group* getGroup(const QString &name) const;
      Link* getLink(const QString &name) const;
      Constraint* getConstraint(const QString &name) const;
      Observer* getObserver(const QString &name) const;
      void setFrame(Frame *frame_, int i) { frame[i] = frame_; }
      void setContour(Contour *contour_, int i) { contour[i] = contour_; }
      void setGroup(Group *group_, int i) { group[i] = group_; }
      void setObject(Object *object_, int i) { object[i] = object_; }
      void setLink(Link *link_, int i) { link[i] = link_; }
      void setConstraint(Constraint *constraint_, int i) { constraint[i] = constraint_; }
      void setObserver(Observer *observer_, int i) { observer[i] = observer_; }
      void addFrame(Frame *frame);
      void addContour(Contour *contour);
      void addGroup(Group *group);
      void addObject(Object *object);
      void addLink(Link *link);
      void addConstraint(Constraint *constraint);
      void addObserver(Observer *observer);
      void removeElement(Element *element);
      ElementPropertyDialog* createPropertyDialog() {return new GroupPropertyDialog(this);}
      QMenu* createFrameContextMenu() {return new FixedRelativeFramesContextMenu(this);}
      QMenu* createContextMenu() { return new GroupContextMenu(this); }
  };

}

#endif
