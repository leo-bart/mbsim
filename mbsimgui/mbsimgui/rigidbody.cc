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
#include "rigidbody.h"
#include "objectfactory.h"
#include "frame.h"
#include "contour.h"
#include "group.h"
#include "basic_properties.h"
#include "kinematics_properties.h"
#include "ombv_properties.h"

using namespace std;
using namespace MBXMLUtils;

RigidBody::RigidBody(const string &str, Element *parent) : Body(str,parent), constrained(false), K(0,false), translation(0,false), rotation(0,false), ombvEditor(0,true), weightArrow(0,false), jointForceArrow(0,false), jointMomentArrow(0,false), isFrameOfBodyForRotation(0,false) {
  Frame *C = new Frame("C",this);
  addFrame(C);

  K.setProperty(new LocalFrameOfReferenceProperty("Frame[C]",this,MBSIMNS"frameForKinematics"));

  vector<PhysicalVariableProperty> input;
  input.push_back(PhysicalVariableProperty(new ScalarProperty("1"),"kg",MBSIMNS"mass"));
  mass.setProperty(new ExtPhysicalVarProperty(input));

  input.clear();
  input.push_back(PhysicalVariableProperty(new MatProperty(getEye<string>(3,3,"0.01","0")),"kg*m^2",MBSIMNS"inertiaTensor"));
  inertia.setProperty(new ExtPhysicalVarProperty(input));

  translation.setProperty(new TranslationChoiceProperty(0,""));
  translation.setXMLName(MBSIMNS"translation");

  rotation.setProperty(new RotationChoiceProperty(2,""));
  rotation.setXMLName(MBSIMNS"rotation");

  ombvEditor.setProperty(new OMBVBodySelectionProperty(this));

  weightArrow.setProperty(new OMBVArrowProperty("NOTSET",getID()));
  weightArrow.setXMLName(MBSIMNS"openMBVWeightArrow",false);

  jointForceArrow.setProperty(new OMBVArrowProperty("NOTSET",getID()));
  jointForceArrow.setXMLName(MBSIMNS"openMBVJointForceArrow",false);

  jointMomentArrow.setProperty(new OMBVArrowProperty("NOTSET",getID()));
  jointMomentArrow.setXMLName(MBSIMNS"openMBVJointMomentArrow",false);

  input.clear();
  input.push_back(PhysicalVariableProperty(new ScalarProperty("0"),"",MBSIMNS"isFrameOfBodyForRotation"));
  isFrameOfBodyForRotation.setProperty(new ExtPhysicalVarProperty(input)); 
}

int RigidBody::getqRelSize() const {
  int nq=0, nqT=0, nqR=0;
  if(translation.isActive()) {
    const TranslationChoiceProperty *trans = static_cast<const TranslationChoiceProperty*>(translation.getProperty());
    if(trans->isIndependent())
      nqT = trans->getqTSize();
    else
      nq = trans->getqSize();
  }
  if(rotation.isActive()) {
    const RotationChoiceProperty *rot = static_cast<const RotationChoiceProperty*>(rotation.getProperty());
    if(rot->isIndependent())
      nqR = rot->getqRSize();
    else
      nq = rot->getqSize();
  }
  if(nq == 0)
    nq = nqT + nqR;
  return nq;
}

int RigidBody::getuRelSize() const {
  int nu=0, nuT=0, nuR=0;
  if(translation.isActive()) {
    const TranslationChoiceProperty *trans = static_cast<const TranslationChoiceProperty*>(translation.getProperty());
    if(trans->isIndependent())
      nuT = trans->getuTSize();
    else
      nu = trans->getuSize();
  }
  if(rotation.isActive()) {
    const RotationChoiceProperty *rot = static_cast<const RotationChoiceProperty*>(rotation.getProperty());
    if(rot->isIndependent())
      nuR = rot->getuRSize();
    else
      nu = rot->getuSize();
  }
  if(nu == 0)
    nu = nuT + nuR;
  return nu;
}

void RigidBody::initialize() {
  Body::initialize();

  for(int i=0; i<frame.size(); i++)
    frame[i]->initialize();
  for(int i=0; i<contour.size(); i++)
    contour[i]->initialize();
}

void RigidBody::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e;
  Body::initializeUsingXML(element);

  // frames
  e=element->FirstChildElement(MBSIMNS"frames")->FirstChildElement();
  while(e && e->ValueStr()==MBSIMNS"frame") {
    TiXmlElement *ec=e->FirstChildElement();
    FixedRelativeFrame *f=new FixedRelativeFrame(ec->Attribute("name"),this);
    addFrame(f);
    f->initializeUsingXML(ec);
    f->initializeUsingXML2(e);
    e=e->NextSiblingElement();
  }
  Frame *f;
  while(e) {
    if(e->ValueStr()==PVNS"embed") {
      TiXmlElement *ee = 0;
      if(e->Attribute("href"))
        f=Frame::readXMLFile(e->Attribute("href"),this);
      else {
        ee = e->FirstChildElement();
        if(ee->ValueStr() == PVNS"localParameter")
          ee = ee->NextSiblingElement();
        f=ObjectFactory::getInstance()->createFrame(ee,this);
      }
      if(f) {
        addFrame(f);
        f->initializeUsingXMLEmbed(e);
        if(ee)
          f->initializeUsingXML(ee);
      }
    }
    else {
      f=ObjectFactory::getInstance()->createFrame(e,this);
      addFrame(f);
      f->initializeUsingXML(e);
    }
    e=e->NextSiblingElement();
  }

  // contours
  e=element->FirstChildElement(MBSIMNS"contours")->FirstChildElement();
  Contour *c;
  while(e && e->ValueStr()==MBSIMNS"contour") {
    TiXmlElement *ec=e->FirstChildElement();
    c=ObjectFactory::getInstance()->createContour(ec,this);
    if(c) {
      addContour(c);
      c->initializeUsingXML(ec);
    }
    FixedRelativeFrame *f=new FixedRelativeFrame("ContourFrame"+toStr(int(contour.size())),this);
    addFrame(f);
    f->initializeUsingXML2(e);
    c->setSavedFrameOfReference(string("../Frame[")+f->getName()+"]");
    e=e->NextSiblingElement();
  }
  while(e) {
    if(e->ValueStr()==PVNS"embed") {
      TiXmlElement *ee = 0;
      if(e->Attribute("href"))
        c=Contour::readXMLFile(e->Attribute("href"),this);
      else {
        ee = e->FirstChildElement();
        if(ee->ValueStr() == PVNS"localParameter")
          ee = ee->NextSiblingElement();
        c=ObjectFactory::getInstance()->createContour(ee,this);
      }
      if(c) {
        addContour(c);
        c->initializeUsingXMLEmbed(e);
        if(ee)
          c->initializeUsingXML(ee);
      }
    }
    else {
      c=ObjectFactory::getInstance()->createContour(e,this);
      if(c) {
        addContour(c);
        c->initializeUsingXML(e);
      }
    }
    e=e->NextSiblingElement();
  }

  K.initializeUsingXML(element);

  mass.initializeUsingXML(element);
  inertia.initializeUsingXML(element);

  translation.initializeUsingXML(element);
  rotation.initializeUsingXML(element);

  isFrameOfBodyForRotation.initializeUsingXML(element);

  ombvEditor.initializeUsingXML(element);

  e=element->FirstChildElement(MBSIMNS"enableOpenMBVFrameC");
  if(e)
    getFrame(0)->initializeUsingXML2(e);
  else
    getFrame(0)->setOpenMBVFrame(false);

  weightArrow.initializeUsingXML(element);

  jointForceArrow.initializeUsingXML(element);
  jointMomentArrow.initializeUsingXML(element);

  Body::initializeUsingXML(element);
}

TiXmlElement* RigidBody::writeXMLFile(TiXmlNode *parent) {

  TiXmlElement *ele0 = Body::writeXMLFile(parent);
  TiXmlElement *ele1;

  K.writeXMLFile(ele0);

  mass.writeXMLFile(ele0);
  inertia.writeXMLFile(ele0);

  translation.writeXMLFile(ele0);
  rotation.writeXMLFile(ele0);

  ele1 = new TiXmlElement( MBSIMNS"frames" );
  for(int i=1; i<frame.size(); i++)
    if(frame[i]->isEmbedded())
      frame[i]->writeXMLFileEmbed(ele1);
    else
      frame[i]->writeXMLFile(ele1);
  ele0->LinkEndChild( ele1 );

  ele1 = new TiXmlElement( MBSIMNS"contours" );
  for(int i=0; i<contour.size(); i++)
    if(contour[i]->isEmbedded())
      contour[i]->writeXMLFileEmbed(ele1);
    else
      contour[i]->writeXMLFile(ele1);
  ele0->LinkEndChild( ele1 );

  isFrameOfBodyForRotation.writeXMLFile(ele0);

  ombvEditor.writeXMLFile(ele0);

  Frame *C = getFrame(0);
  if(C->openMBVFrame()) {
    ele1 = new TiXmlElement( MBSIMNS"enableOpenMBVFrameC" );
    C->writeXMLFile2(ele1);
    ele0->LinkEndChild(ele1);
  }

  weightArrow.writeXMLFile(ele0);

  jointForceArrow.writeXMLFile(ele0);
  jointMomentArrow.writeXMLFile(ele0);

  return ele0;
}