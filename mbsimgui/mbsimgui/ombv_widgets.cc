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
#include "ombv_widgets.h"
#include "variable_widgets.h"
#include "extended_widgets.h"
#include "body.h"
#include "frame.h"
#include "mainwindow.h"
#include <QtGui>
#include <xercesc/dom/DOMProcessingInstruction.hpp>

using namespace std;
using namespace MBXMLUtils;
using namespace xercesc;

namespace MBSimGUI {

  extern MainWindow *mw;

  OMBVRigidBodyWidgetFactory::OMBVRigidBodyWidgetFactory() {
    name.push_back("Cube");
    name.push_back("Cuboid");
    name.push_back("Frustum");
    name.push_back("Extrusion");
    name.push_back("Sphere");
    name.push_back("IvBody");
    name.push_back("CompoundRigidBody");
    name.push_back("InvisibleBody");
    xmlName.push_back(OPENMBV%"Cube");
    xmlName.push_back(OPENMBV%"Cuboid");
    xmlName.push_back(OPENMBV%"Frustum");
    xmlName.push_back(OPENMBV%"Extrusion");
    xmlName.push_back(OPENMBV%"Sphere");
    xmlName.push_back(OPENMBV%"IvBody");
    xmlName.push_back(OPENMBV%"CompoundRigidBody");
    xmlName.push_back(OPENMBV%"InvisibleBody");
  }

  QWidget* OMBVRigidBodyWidgetFactory::createWidget(int i) {
    if(i==0)
      return new CubeWidget("Cube",OPENMBV%"Cube");
    if(i==1)
      return new CuboidWidget("Cuboid",OPENMBV%"Cuboid");
    if(i==2)
      return new FrustumWidget("Frustum",OPENMBV%"Frustum");
    if(i==3)
      return new ExtrusionWidget("Extrusion",OPENMBV%"Extrusion");
    if(i==4)
      return new SphereWidget("Sphere",OPENMBV%"Sphere");
    if(i==5)
      return new IvBodyWidget("IvBody",OPENMBV%"IvBody");
    if(i==6)
      return new CompoundRigidBodyWidget("CompoundRigidBody",OPENMBV%"CompoundRigidBody");
    if(i==7)
      return new InvisibleBodyWidget("InvisibleBody",OPENMBV%"InvisibleBody");
    return NULL;
  }

  DOMElement* OMBVObjectWidget::initializeUsingXML(DOMElement *element) {
//    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
//    if(e) {
//      diffuseColor->initializeUsingXML(e);
//      transparency->initializeUsingXML(e);
//    }
    return element;
  }

  DOMElement* OMBVObjectWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *newele;
    if(xmlName!=FQN()) {
    DOMDocument *doc = parent->getOwnerDocument();
    newele=D(doc)->createElement(xmlName);
    E(newele)->setAttribute("name",name.toStdString());
    DOMElement *ele = E(static_cast<DOMElement*>(parent))->getFirstElementChildNamed(xmlName);
    if(ele)
      parent->replaceChild(newele,ele);
    else
      parent->insertBefore(newele,ref);
    }
    else
      newele = (DOMElement*)parent;
    return newele;
  }

  //class OmbvBodyWidgetFactory : public WidgetFactory {
  //  public:
  //    OmbvBodyWidgetFactory() { }
  //    Widget* createWidget(int i);
  //};
  //
  //Widget* OmbvBodyWidgetFactory::createWidget(int i) {
  //
  //  vector<QWidget*> widget;
  //  vector<QString> name;
  //  widget.push_back(new CubeWidget); name.push_back("Cube");
  //  widget.push_back(new CuboidWidget); name.push_back("Cuboid");
  //  widget.push_back(new FrustumWidget); name.push_back("Frustum");
  //  widget.push_back(new SphereWidget); name.push_back("Sphere");
  //  widget.push_back(new IvBodyWidget); name.push_back("IvBody");
  //  widget.push_back(new InvisibleBodyWidget); name.push_back("InvisibleBody");
  //  return new ChoiceWidget(widget,name);
  //}

  MBSOMBVWidget::MBSOMBVWidget(const QString &name, const FQN &xmlName_) : OMBVObjectWidget(name), xmlName(xmlName_) {
    QVBoxLayout *layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

//    diffuseColor = new ExtWidget("Diffuse color",new ColorWidget,true,true,MBSIM%"diffuseColor");
//    layout->addWidget(diffuseColor);
//
    transparency = new ExtWidget("Transparency",new ChoiceWidget2(new ScalarWidgetFactory("0.3",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"transparency");
    layout->addWidget(transparency);
  }

  DOMElement* MBSOMBVWidget::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      diffuseColor->initializeUsingXML(e);
      transparency->initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* MBSOMBVWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=initXMLFile(parent,ref);
    writeProperties(e);
    return e;
  }

  DOMElement* MBSOMBVWidget::initXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *newele;
    if(xmlName!=FQN()) {
    DOMDocument *doc = parent->getOwnerDocument();
    newele=D(doc)->createElement(xmlName);
    DOMElement *ele = E(static_cast<DOMElement*>(parent))->getFirstElementChildNamed(xmlName);
    if(ele)
      parent->replaceChild(newele,ele);
    else
      parent->insertBefore(newele,ref);
    }
    else
      newele = (DOMElement*)parent;
    return newele;
  }

  DOMElement* MBSOMBVWidget::writeProperties(DOMElement *e) {
    diffuseColor->writeXMLFile(e);
    transparency->writeXMLFile(e);
    return e;
  }

  PointMBSOMBVWidget::PointMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0.001"), lengthUnits(), 4));
    size = new ExtWidget("Size",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(size);
  }

  LineMBSOMBVWidget::LineMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), lengthUnits(), 4));
    length = new ExtWidget("Size",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(length);
  }

  PlaneMBSOMBVWidget::PlaneMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new VecWidget(getScalars<QString>(2,"1")), lengthUnits(), 4));
    length = new ExtWidget("Size",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(length);
  }

  PlanarContourMBSOMBVWidget::PlanarContourMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new VecSizeVarWidget(2,1,100), noUnitUnits(), 4));
    nodes = new ExtWidget("Nodes",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(nodes);
  }

  SpatialContourMBSOMBVWidget::SpatialContourMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new VecSizeVarWidget(2,1,100), noUnitUnits(), 4));
    etaNodes = new ExtWidget("Eta nodes",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(etaNodes);
    input.clear();
    input.push_back(new PhysicalVariableWidget(new VecSizeVarWidget(2,1,100), noUnitUnits(), 4));
    xiNodes = new ExtWidget("Xi nodes",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(xiNodes);
  }

  ArrowMBSOMBVWidget::ArrowMBSOMBVWidget(const QString &name, bool fromPoint) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), noUnitUnits(), 1));
    scaleLength = new ExtWidget("Scale length",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(scaleLength);

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), noUnitUnits(), 1));
    scaleSize = new ExtWidget("Scale size",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(scaleSize);

    vector<QString> list;
    list.push_back("toPoint");
    list.push_back("fromPoint");
    list.push_back("midPoint");
    referencePoint = new ExtWidget("Reference point",new TextChoiceWidget(list,fromPoint?1:0),true);
    if(fromPoint)
      referencePoint->setChecked(true);
    layout()->addWidget(referencePoint);
  }

  CoilSpringMBSOMBVWidget::CoilSpringMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    vector<PhysicalVariableWidget*> input;
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("3"), QStringList(), 1));
    numberOfCoils= new ExtWidget("Number of coils",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(numberOfCoils);

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), lengthUnits(), 4));
    springRadius= new ExtWidget("Spring radius",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(springRadius);

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("-1"), lengthUnits(), 4));
    crossSectionRadius = new ExtWidget("Cross section radius",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(crossSectionRadius);

    input.clear();
    input.push_back(new PhysicalVariableWidget(new ScalarWidget("-1"), lengthUnits(), 4));
    nominalLength= new ExtWidget("Nominal length",new ExtPhysicalVarWidget(input),true);
    layout()->addWidget(nominalLength);

    vector<QString> list;
    list.push_back("tube");
    list.push_back("scaledTube");
    list.push_back("polyline");
    type = new ExtWidget("Type",new TextChoiceWidget(list,0),true);
    layout()->addWidget(type);

    minCol = new ExtWidget("Minimal color value",new ChoiceWidget2(new ScalarWidgetFactory("0",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(minCol);
    maxCol = new ExtWidget("Maximal color value",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(maxCol);
  }

  FrameMBSOMBVWidget::FrameMBSOMBVWidget(const QString &name, const FQN &xmlName) : MBSOMBVWidget(name,xmlName) {
    size = new ExtWidget("Size",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"size");
    size->setToolTip("Set the size of the frame");
    layout()->addWidget(size);

    offset = new ExtWidget("Offset",new ChoiceWidget2(new ScalarWidgetFactory("0",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft,5),true,false,MBSIM%"offset");
    offset->setToolTip("Set the offset of the frame");
    layout()->addWidget(offset);
  }

  DOMElement* FrameMBSOMBVWidget::initializeUsingXML(DOMElement *element) {
    DOMElement *e=(xmlName==FQN())?element:E(element)->getFirstElementChildNamed(xmlName);
    if(e) {
      size->initializeUsingXML(e);
      offset->initializeUsingXML(e);
      transparency->initializeUsingXML(e);
    }
    return e;
  }

  DOMElement* FrameMBSOMBVWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    if(xmlName!=FQN()) {
      DOMElement *e=MBSOMBVWidget::initXMLFile(parent,ref);
      size->writeXMLFile(e);
      offset->writeXMLFile(e);
      transparency->writeXMLFile(e);
      return e;
    }
    else {
      size->writeXMLFile(parent);
      offset->writeXMLFile(parent);
      transparency->writeXMLFile(parent);
      return 0;
    }
  }

  OMBVDynamicColoredObjectWidget::OMBVDynamicColoredObjectWidget(const QString &name, const FQN &xmlName) : OMBVObjectWidget(name,xmlName) {
    layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

//    vector<PhysicalVariableWidget*> input;
//    input.push_back(new PhysicalVariableWidget(new ScalarWidget("0"), noUnitUnits(), 1));
//    minimalColorValue = new ExtWidget("Minimal color value",new ExtPhysicalVarWidget(input),true);
//    layout->addWidget(minimalColorValue);
//
//    input.clear();
//    input.push_back(new PhysicalVariableWidget(new ScalarWidget("1"), noUnitUnits(), 1));
//    maximalColorValue = new ExtWidget("Maximal color value",new ExtPhysicalVarWidget(input),true);
//    layout->addWidget(maximalColorValue);
//
//    diffuseColor = new ExtWidget("Diffuse color",new ColorWidget,true);
//    layout->addWidget(diffuseColor);
//
    transparency = new ExtWidget("Transparency",new ChoiceWidget2(new ScalarWidgetFactory("0.3",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft,5),true,true,OPENMBV%"transparency");
    layout->addWidget(transparency);
  }

  DOMElement* OMBVDynamicColoredObjectWidget::initializeUsingXML(DOMElement *element) {
    OMBVObjectWidget::initializeUsingXML(element);
    transparency->initializeUsingXML(element);
    return element;
  }

  DOMElement* OMBVDynamicColoredObjectWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVObjectWidget::writeXMLFile(parent);
    transparency->writeXMLFile(e);
    return e;
  }

  OMBVRigidBodyWidget::OMBVRigidBodyWidget(const QString &name, const FQN &xmlName) : OMBVDynamicColoredObjectWidget(name,xmlName) {

    transparency->setActive(true);

    trans = new ExtWidget("Initial translation",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"initialTranslation");
    layout->addWidget(trans);

    rot = new ExtWidget("Initial rotation",new ChoiceWidget2(new VecWidgetFactory(3,vector<QStringList>(3,angleUnits())),QBoxLayout::RightToLeft),false,false,OPENMBV%"initialRotation");
    layout->addWidget(rot);
//
    scale = new ExtWidget("Scale factor",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"scaleFactor");
    layout->addWidget(scale);
  }

  DOMElement* OMBVRigidBodyWidget::initializeUsingXML(DOMElement *element) {
    OMBVDynamicColoredObjectWidget::initializeUsingXML(element);
    trans->initializeUsingXML(element);
    rot->initializeUsingXML(element);
    scale->initializeUsingXML(element);
    return element;
  }

  DOMElement* OMBVRigidBodyWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVDynamicColoredObjectWidget::writeXMLFile(parent);
    trans->writeXMLFile(e);
    rot->writeXMLFile(e);
    scale->writeXMLFile(e);
    return e;
  }

  CubeWidget::CubeWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name,xmlName) {

    length = new ExtWidget("Length",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"length");
    layout->addWidget(length);
  }

  DOMElement* CubeWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    length->initializeUsingXML(element);
    return element;
  }

  DOMElement* CubeWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    length->writeXMLFile(e);
    return e;
  }

  CuboidWidget::CuboidWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name, xmlName) {

    length = new ExtWidget("Length",new ChoiceWidget2(new VecWidgetFactory(strToVec(QString("[1;1;1]")),vector<QStringList>(3,lengthUnits()),vector<int>(3,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"length");
    layout->addWidget(length);
  }

  DOMElement* CuboidWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    length->initializeUsingXML(element);
    return element;
  }

  DOMElement* CuboidWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    length->writeXMLFile(e);
    return e;
  }

  SphereWidget::SphereWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name,xmlName) {

    radius = new ExtWidget("Radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"radius");
  }

  DOMElement* SphereWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    radius->initializeUsingXML(element);
    return element;
  }

  DOMElement* SphereWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    radius->writeXMLFile(e);
    return e;
  }

  FrustumWidget::FrustumWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name,xmlName) {
    vector<QString> r(3);
    r[2] = "0.5";
    static_cast<VecWidget*>(static_cast<PhysicalVariableWidget*>(static_cast<ChoiceWidget2*>(trans->getWidget())->getWidget())->getWidget())->setVec(r);

    base = new ExtWidget("Base radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"baseRadius");
    layout->addWidget(base);

    top = new ExtWidget("Top radius",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"topRadius");
    layout->addWidget(top);

    height = new ExtWidget("Height",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"height");
    layout->addWidget(height);

    innerBase = new ExtWidget("Inner base radius",new ChoiceWidget2(new ScalarWidgetFactory("0",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"innerBaseRadius");
    layout->addWidget(innerBase);

    innerTop = new ExtWidget("Inner top radius",new ChoiceWidget2(new ScalarWidgetFactory("0",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft,5),false,false,OPENMBV%"innerTopRadius");
    layout->addWidget(innerTop);
  }

  DOMElement* FrustumWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    base->initializeUsingXML(element);
    top->initializeUsingXML(element);
    height->initializeUsingXML(element);
    innerBase->initializeUsingXML(element);
    innerTop->initializeUsingXML(element);
    return element;
  }

  DOMElement* FrustumWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    base->writeXMLFile(e);
    top->writeXMLFile(e);
    height->writeXMLFile(e);
    innerBase->writeXMLFile(e);
    innerTop->writeXMLFile(e);
    return e;
  }

  ExtrusionWidget::ExtrusionWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name,xmlName) {
    vector<QString> list;
    list.push_back("odd");
    list.push_back("nonzero");
    list.push_back("positive");
    list.push_back("negative");
    list.push_back("absGEqTwo");
    windingRule = new ExtWidget("Winding rule",new TextChoiceWidget(list,0));
    layout->addWidget(windingRule);

    height = new ExtWidget("Height",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,lengthUnits()),vector<int>(2,4)),QBoxLayout::RightToLeft));
    layout->addWidget(height);

    contour = new ExtWidget("Contour",new ChoiceWidget2(new MatRowsVarWidgetFactory(3,3,vector<QStringList>(3,lengthUnits()),vector<int>(3,2)),QBoxLayout::RightToLeft));
    layout->addWidget(contour);
  }

  DOMElement* ExtrusionWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    windingRule->initializeUsingXML(element);
    height->initializeUsingXML(element);
    contour->initializeUsingXML(element);
    return element;
  }

  DOMElement* ExtrusionWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    windingRule->writeXMLFile(e);
    height->writeXMLFile(e);
    contour->writeXMLFile(e);
    return e;
  }

  IvBodyWidget::IvBodyWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name,xmlName) {

    ivFileName = new ExtWidget("Iv file name",new FileWidget("XML model files", "iv files (*.iv *.wrl)"));
    layout->addWidget(ivFileName);

    creaseEdges = new ExtWidget("Crease edges",new ChoiceWidget2(new ScalarWidgetFactory("-1",vector<QStringList>(2,angleUnits())),QBoxLayout::RightToLeft),false,false,OPENMBV%"creaseEdges");
    layout->addWidget(creaseEdges);

    boundaryEdges = new ExtWidget("Boundary edges",new ChoiceWidget2(new BoolWidgetFactory("0"),QBoxLayout::RightToLeft),false,false,OPENMBV%"boundaryEdges");
    layout->addWidget(boundaryEdges);
  }

  DOMElement* IvBodyWidget::initializeUsingXML(DOMElement *element) {
    OMBVRigidBodyWidget::initializeUsingXML(element);
    ivFileName->initializeUsingXML(element);
    creaseEdges->initializeUsingXML(element);
    boundaryEdges->initializeUsingXML(element);
    return element;
  }

  DOMElement* IvBodyWidget::writeXMLFile(DOMNode *parent, xercesc::DOMNode *ref) {
    DOMElement *e=OMBVRigidBodyWidget::writeXMLFile(parent);
    ivFileName->writeXMLFile(e);
    creaseEdges->writeXMLFile(e);
    boundaryEdges->writeXMLFile(e);
    return e;
  }

  CompoundRigidBodyWidget::CompoundRigidBodyWidget(const QString &name, const FQN &xmlName) : OMBVRigidBodyWidget(name) {
    bodies = new ExtWidget("Bodies",new ListWidget(new ChoiceWidgetFactory(new OMBVRigidBodyWidgetFactory),"Body",1,1));
    layout->addWidget(bodies);
  }

  FlexibleBodyFFRMBSOMBVWidget::FlexibleBodyFFRMBSOMBVWidget(const QString &name) : MBSOMBVWidget(name) {
    minCol = new ExtWidget("Minimal color value",new ChoiceWidget2(new ScalarWidgetFactory("0",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(minCol);
    maxCol = new ExtWidget("Maximal color value",new ChoiceWidget2(new ScalarWidgetFactory("1",vector<QStringList>(2,noUnitUnits()),vector<int>(2,1)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(maxCol);
    nodes = new ExtWidget("Nodes",new ChoiceWidget2(new VecSizeVarWidgetFactory(1,vector<QStringList>(3)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(nodes);
    indices = new ExtWidget("Indices",new ChoiceWidget2(new VecSizeVarWidgetFactory(1,vector<QStringList>(3)),QBoxLayout::RightToLeft),true);
    layout()->addWidget(indices);
  }


}
