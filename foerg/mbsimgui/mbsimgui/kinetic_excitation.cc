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

#include <config.h>
#include "kinetic_excitation.h"
#include "ombv_properties.h"
#include "kinetics_properties.h"
#include "kinetics_widgets.h"
#include "extended_widgets.h"
#include "ombv_widgets.h"

using namespace std;

KineticExcitation::KineticExcitation(const QString &str, TreeItem *parentItem) : Link(str, parentItem), forceArrow(0,true), momentArrow(0,true), force(0,false), moment(0,false), frameOfReference(0,false) {

  forceArrow.setProperty(new OMBVArrowProperty("NOTSET"));
  ((OMBVArrowProperty*)forceArrow.getProperty())->setID(getID());

  momentArrow.setProperty(new OMBVArrowProperty("NOTSET"));
  ((OMBVArrowProperty*)momentArrow.getProperty())->setID(getID());

  vector<Property*> widget;
  widget.push_back(new ConnectFramesProperty(1,this));
  widget.push_back(new ConnectFramesProperty(2,this));

  connections.setProperty(new PropertyChoiceProperty(widget)); 

  force.setProperty(new ForceChoiceProperty(forceArrow,MBSIMNS"force"));
  moment.setProperty(new ForceChoiceProperty(momentArrow,MBSIMNS"moment"));

  frameOfReference.setProperty(new FrameOfReferenceProperty(0,this,MBSIMNS"frameOfReference"));

}

KineticExcitation::~KineticExcitation() {
}

void KineticExcitation::initialize() {
  Link::initialize();
  connections.initialize();
}

void KineticExcitation::initializeUsingXML(TiXmlElement *element) {
  Link::initializeUsingXML(element);
  frameOfReference.initializeUsingXML(element);
  force.initializeUsingXML(element);
  moment.initializeUsingXML(element);
  connections.initializeUsingXML(element);
}

TiXmlElement* KineticExcitation::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele0 = Link::writeXMLFile(parent);
  frameOfReference.writeXMLFile(ele0);
  force.writeXMLFile(ele0);
  moment.writeXMLFile(ele0);
  connections.writeXMLFile(ele0);
  return ele0;
}

