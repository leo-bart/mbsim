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
#include "kinematics_properties.h"
#include "frame.h"
#include "string_properties.h"
#include "function_properties.h"
#include "extended_properties.h"
#include "basic_widgets.h"
#include "string_widgets.h"
#include "kinematics_widgets.h"
#include "extended_widgets.h"
#include "octaveutils.h"
#include <mbxmlutilstinyxml/tinyxml.h>
#include <mbxmlutilstinyxml/tinynamespace.h>

using namespace std;

LinearTranslationProperty::LinearTranslationProperty() {
  vector<PhysicalStringProperty*> input;
  input.push_back(new PhysicalStringProperty(new MatProperty(3,1),"-",MBSIMNS"translationVectors"));
  mat.setProperty(new ExtPhysicalVarProperty(input));
}

int LinearTranslationProperty::getSize() const {
  string str = evalOctaveExpression(static_cast<const ExtPhysicalVarProperty*>(mat.getProperty())->getCurrentPhysicalStringProperty()->getValue());
  vector<vector<string> > A = strToMat(str);
  return A.size()?A[0].size():0;
}

TiXmlElement* LinearTranslationProperty::initializeUsingXML(TiXmlElement *element) {
  return mat.initializeUsingXML(element);
}

TiXmlElement* LinearTranslationProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"LinearTranslation" );
  mat.writeXMLFile(ele2);
  parent->LinkEndChild(ele2);
  return ele2;
}

void LinearTranslationProperty::fromWidget(QWidget *widget) {
  mat.fromWidget(static_cast<LinearTranslationWidget*>(widget)->mat);
}

void LinearTranslationProperty::toWidget(QWidget *widget) {
  mat.toWidget(static_cast<LinearTranslationWidget*>(widget)->mat);
}

TimeDependentTranslationProperty::TimeDependentTranslationProperty() {
  function.setProperty(new Function1ChoiceProperty(MBSIMNS"position"));
}

TiXmlElement* TimeDependentTranslationProperty::initializeUsingXML(TiXmlElement *element) {
  return function.initializeUsingXML(element);
}

TiXmlElement* TimeDependentTranslationProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"TimeDependentTranslation" );
  function.writeXMLFile(ele2);
  parent->LinkEndChild(ele2);
  return ele2;
}

void TimeDependentTranslationProperty::fromWidget(QWidget *widget) {
  function.fromWidget(static_cast<TimeDependentTranslationWidget*>(widget)->function);
}

void TimeDependentTranslationProperty::toWidget(QWidget *widget) {
  function.toWidget(static_cast<TimeDependentTranslationWidget*>(widget)->function);
}

void TranslationChoiceProperty::defineTranslation(int index_) {
  index = index_;
  delete translation;
  if(index==0)
    translation = new LinearTranslationProperty;  
  else if(index==1)
    translation = new TimeDependentTranslationProperty;  
}

TiXmlElement* TranslationChoiceProperty::initializeUsingXML(TiXmlElement *element) {
   TiXmlElement *e=(xmlName=="")?element:element->FirstChildElement(xmlName);
  if(e) {
    TiXmlElement *ee = e->FirstChildElement();
    if(ee) {
      if(ee->ValueStr() == MBSIMNS"LinearTranslation")
        index = 0;
      else if(ee->ValueStr() == MBSIMNS"TimeDependentTranslation")
        index = 1;
      defineTranslation(index);
      translation->initializeUsingXML(ee);
      return e;
    }
  }
  return 0;
}

TiXmlElement* TranslationChoiceProperty::writeXMLFile(TiXmlNode *parent) {
  if(xmlName!="") {
    TiXmlElement *ele0 = new TiXmlElement(xmlName);
    //if(getTranslation()==1) {
    translation->writeXMLFile(ele0);
    //}
    parent->LinkEndChild(ele0);
  }
  else
    translation->writeXMLFile(parent);

 return 0;
}

void TranslationChoiceProperty::fromWidget(QWidget *widget) {
  defineTranslation(static_cast<TranslationChoiceWidget*>(widget)->comboBox->currentIndex());
  translation->fromWidget(static_cast<TranslationChoiceWidget*>(widget)->translation);
}

void TranslationChoiceProperty::toWidget(QWidget *widget) {
  static_cast<TranslationChoiceWidget*>(widget)->comboBox->blockSignals(true);
  static_cast<TranslationChoiceWidget*>(widget)->comboBox->setCurrentIndex(index);
  static_cast<TranslationChoiceWidget*>(widget)->comboBox->blockSignals(false);
  static_cast<TranslationChoiceWidget*>(widget)->blockSignals(true);
  static_cast<TranslationChoiceWidget*>(widget)->defineTranslation(index);
  static_cast<TranslationChoiceWidget*>(widget)->blockSignals(false);
  translation->toWidget(static_cast<TranslationChoiceWidget*>(widget)->translation);
}

TiXmlElement* RotationAboutXAxisProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"RotationAboutXAxis" );
  parent->LinkEndChild(ele2);
  return ele2;
}

TiXmlElement* RotationAboutYAxisProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"RotationAboutYAxis" );
  parent->LinkEndChild(ele2);
  return ele2;
}

TiXmlElement* RotationAboutZAxisProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"RotationAboutZAxis" );
  parent->LinkEndChild(ele2);
  return ele2;
}

RotationAboutFixedAxisProperty::RotationAboutFixedAxisProperty() {
  vector<PhysicalStringProperty*> input;
  input.push_back(new PhysicalStringProperty(new VecProperty(3),"-",MBSIMNS"axisOfRotation"));
  vec.setProperty(new ExtPhysicalVarProperty(input));  
}

TiXmlElement* RotationAboutFixedAxisProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"RotationAboutFixedAxis" );
  vec.writeXMLFile(ele2);
  parent->LinkEndChild(ele2);
  return ele2;
}

void RotationAboutFixedAxisProperty::fromWidget(QWidget *widget) {
  vec.fromWidget(static_cast<RotationAboutFixedAxisWidget*>(widget)->vec);
}

void RotationAboutFixedAxisProperty::toWidget(QWidget *widget) {
  vec.toWidget(static_cast<RotationAboutFixedAxisWidget*>(widget)->vec);
}

TiXmlElement* RotationAboutFixedAxisProperty::initializeUsingXML(TiXmlElement *element) {
  return vec.initializeUsingXML(element);
}

TiXmlElement* RotationAboutAxesXYProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"RotationAboutAxesXY" );
  parent->LinkEndChild(ele2);
  return ele2;
}

TiXmlElement* CardanAnglesProperty::writeXMLFile(TiXmlNode *parent) {
  TiXmlElement *ele2 = new TiXmlElement( MBSIMNS"CardanAngles" );
  parent->LinkEndChild(ele2);
  return ele2;
}

TiXmlElement* RotationChoiceProperty::initializeUsingXML(TiXmlElement *element) {
  TiXmlElement *e=(xmlName=="")?element:element->FirstChildElement(xmlName);
  if(e) {
    TiXmlElement *ee = e->FirstChildElement();
    if(ee) {
      if(ee->ValueStr() == MBSIMNS"RotationAboutXAxis")
        index = 0;
      else if(ee->ValueStr() == MBSIMNS"RotationAboutYAxis")
        index = 1;
      else if(ee->ValueStr() == MBSIMNS"RotationAboutZAxis")
        index = 2;
      else if(ee->ValueStr() == MBSIMNS"RotationAboutFixedAxis")
        index = 3;
      else if(ee->ValueStr() == MBSIMNS"CardanAngles")
        index = 4;
      else if(ee->ValueStr() == MBSIMNS"RotationAboutAxesXY")
        index = 5;
      defineRotation(index);
      rotation->initializeUsingXML(ee);
      return e;
    }
  }
  return 0;
}

TiXmlElement* RotationChoiceProperty::writeXMLFile(TiXmlNode *parent) {
  if(xmlName!="") {
    TiXmlElement *ele0 = new TiXmlElement( MBSIMNS"rotation" );
    rotation->writeXMLFile(ele0);
    parent->LinkEndChild(ele0);
  }
  else
    rotation->writeXMLFile(parent);

 return 0;
}

void RotationChoiceProperty::defineRotation(int index_) {
  index = index_;
  delete rotation;
  if(index==0)
    rotation = new RotationAboutXAxisProperty;  
  else if(index==1)
    rotation = new RotationAboutYAxisProperty;  
  else if(index==2)
    rotation = new RotationAboutZAxisProperty;  
  else if(index==3)
    rotation = new RotationAboutFixedAxisProperty;  
  else if(index==4)
    rotation = new CardanAnglesProperty;  
  else if(index==5)
    rotation = new RotationAboutAxesXYProperty;  
}

void RotationChoiceProperty::fromWidget(QWidget *widget) {
  defineRotation(static_cast<RotationChoiceWidget*>(widget)->comboBox->currentIndex());
  rotation->fromWidget(static_cast<RotationChoiceWidget*>(widget)->rotation);
}

void RotationChoiceProperty::toWidget(QWidget *widget) {
  static_cast<RotationChoiceWidget*>(widget)->comboBox->blockSignals(true);
  static_cast<RotationChoiceWidget*>(widget)->comboBox->setCurrentIndex(index);
  static_cast<RotationChoiceWidget*>(widget)->comboBox->blockSignals(false);
  static_cast<RotationChoiceWidget*>(widget)->blockSignals(true);
  static_cast<RotationChoiceWidget*>(widget)->defineRotation(index);
  static_cast<RotationChoiceWidget*>(widget)->blockSignals(false);
  rotation->toWidget(static_cast<RotationChoiceWidget*>(widget)->rotation);
}