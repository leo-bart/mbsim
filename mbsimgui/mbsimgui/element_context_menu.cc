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
#include "element_context_menu.h"
#include "mainwindow.h"
#include "rigid_body.h"
#include "flexible_body_ffr.h"
#include "constraint.h"
#include "kinetic_excitation.h"
#include "spring_damper.h"
#include "joint.h"
#include "friction.h"
#include "contact.h"
#include "observer.h"
#include "frame.h"
#include "contour.h"
#include "group.h"
#include "gear.h"
#include "connection.h"
#include "sensor.h"
#include "element_view.h"

namespace MBSimGUI {

  extern MainWindow *mw;

  ElementContextMenu::ElementContextMenu(Element *element_, QWidget *parent, bool removable) : QMenu(parent), element(element_) {
    QAction *action=new QAction("Edit", this);
    connect(action,SIGNAL(triggered()),mw->getElementList(),SLOT(openEditor()));
    addAction(action);
    if(removable) {
      addSeparator();
      action=new QAction("Copy", this);
      connect(action,SIGNAL(triggered()),mw,SLOT(copyElement()));
      addAction(action);
      action=new QAction("Cut", this);
      connect(action,SIGNAL(triggered()),mw,SLOT(cutElement()));
      addAction(action);
      action=new QAction("Save as", this);
      connect(action,SIGNAL(triggered()),mw,SLOT(saveElementAs()));
      addAction(action);
      addSeparator();
      action=new QAction("Remove", this);
      connect(action,SIGNAL(triggered()),mw,SLOT(removeElement()));
      addAction(action);
      addSeparator();
    }
  }

  FrameContextMenu::FrameContextMenu(Frame *frame, QWidget *parent, bool removable) : ElementContextMenu(frame,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(frame->getParent()->getIndexOfFrame(frame)>1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpFrame()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(frame->getParent()->getIndexOfFrame(frame)<frame->getParent()->getNumberOfFrames()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownFrame()));
    addAction(action);
  }

  ContourContextMenu::ContourContextMenu(Contour *contour, QWidget *parent, bool removable) : ElementContextMenu(contour,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(contour->getParent()->getIndexOfContour(contour)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpContour()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(contour->getParent()->getIndexOfContour(contour)<contour->getParent()->getNumberOfContours()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownContour()));
    addAction(action);
  }

  GroupContextMenu::GroupContextMenu(Group *group, QWidget *parent, bool removable) : ElementContextMenu(group,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(group->getParent()->getIndexOfGroup(group)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpGroup()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(group->getParent()->getIndexOfGroup(group)<group->getParent()->getNumberOfGroups()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownGroup()));
    addAction(action);
  }

  ObjectContextMenu::ObjectContextMenu(Object *object, QWidget *parent, bool removable) : ElementContextMenu(object,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(object->getParent()->getIndexOfObject(object)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpObject()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(object->getParent()->getIndexOfObject(object)<object->getParent()->getNumberOfObjects()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownObject()));
    addAction(action);
  }

  LinkContextMenu::LinkContextMenu(Link *link, QWidget *parent, bool removable) : ElementContextMenu(link,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(link->getParent()->getIndexOfLink(link)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpLink()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(link->getParent()->getIndexOfLink(link)<link->getParent()->getNumberOfLinks()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownLink()));
    addAction(action);
  }

  ConstraintContextMenu::ConstraintContextMenu(Constraint *constraint, QWidget *parent, bool removable) : ElementContextMenu(constraint,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(constraint->getParent()->getIndexOfConstraint(constraint)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpConstraint()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(constraint->getParent()->getIndexOfConstraint(constraint)<constraint->getParent()->getNumberOfConstraints()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownConstraint()));
    addAction(action);
  }

  ObserverContextMenu::ObserverContextMenu(Observer *observer, QWidget *parent, bool removable) : ElementContextMenu(observer,parent,removable) {
    addSeparator();
    QAction *action=new QAction("Move up", this);
    action->setEnabled(observer->getParent()->getIndexOfObserver(observer)>0);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveUpObserver()));
    addAction(action);
    action=new QAction("Move down", this);
    action->setEnabled(observer->getParent()->getIndexOfObserver(observer)<observer->getParent()->getNumberOfObservers()-1);
    connect(action,SIGNAL(triggered()),mw,SLOT(moveDownObserver()));
    addAction(action);
  }

  FramesContextMenu::FramesContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Frame*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
  }

  void FramesContextMenu::paste() {
    mw->loadFrame(element, mw->getElementBuffer().first);
  }

  void FramesContextMenu::load() {
    mw->loadFrame(element);
  }

  FixedRelativeFramesContextMenu::FixedRelativeFramesContextMenu(Element *element, const QString &title, QWidget *parent) : FramesContextMenu(element,title,parent) {
    addSeparator();
    QAction *action = new QAction("Add fixed relative frame", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addFixedRelativeFrame()));
    addAction(action);
  }

  void FixedRelativeFramesContextMenu::addFixedRelativeFrame() {
    mw->addFrame(new FixedRelativeFrame("P"), element);
  }

  NodeFramesContextMenu::NodeFramesContextMenu(Element *element, const QString &title, QWidget *parent) : FramesContextMenu(element,title,parent) {
    addSeparator();
    QAction *action = new QAction("Add node frame", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addNodeFrame()));
    addAction(action);
  }

  void NodeFramesContextMenu::addNodeFrame() {
    mw->addFrame(new NodeFrame("P"), element);
  }

  ContoursContextMenu::ContoursContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Contour*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    action = new QAction("Add point", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addPoint()));
    addAction(action);
    action = new QAction("Add line", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addLine()));
    addAction(action);
    action = new QAction("Add plane", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addPlane()));
    addAction(action);
    action = new QAction("Add sphere", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addSphere()));
    addAction(action);
    action = new QAction("Add circle", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addCircle()));
    addAction(action);
    action = new QAction("Add cuboid", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addCuboid()));
    addAction(action);
    action = new QAction("Add line segment", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addLineSegment()));
    addAction(action);
    action = new QAction("Add planar contour", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addPlanarContour()));
    addAction(action);
    action = new QAction("Add spatial contour", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addSpatialContour()));
    addAction(action);
  }

  void ContoursContextMenu::paste() {
    mw->loadContour(element, mw->getElementBuffer().first);
  }

  void ContoursContextMenu::load() {
    mw->loadContour(element);
  }

  void ContoursContextMenu::addPoint() {
    mw->addContour(new Point("Point"), element);
  }

  void ContoursContextMenu::addLine() {
    mw->addContour(new Line("Line"), element);
  }

  void ContoursContextMenu::addPlane() {
    mw->addContour(new Plane("Plane"), element);
  }

  void ContoursContextMenu::addSphere() {
    mw->addContour(new Sphere("Sphere"), element);
  }

  void ContoursContextMenu::addCircle() {
    mw->addContour(new Circle("Circle"), element);
  }

  void ContoursContextMenu::addCuboid() {
    mw->addContour(new Cuboid("Cuboid"), element);
  }

  void ContoursContextMenu::addLineSegment() {
    mw->addContour(new LineSegment("LineSegment"), element);
  }

  void ContoursContextMenu::addPlanarContour() {
    mw->addContour(new PlanarContour("PlanarContour"), element);
  }

  void ContoursContextMenu::addSpatialContour() {
    mw->addContour(new SpatialContour("SpatialContour"), element);
  }

  GroupsContextMenu::GroupsContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Group*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    action = new QAction("Add", this);
    connect(action,SIGNAL(triggered()),this,SLOT(add()));
    addAction(action);
  }

  void GroupsContextMenu::paste() {
    mw->loadGroup(element, mw->getElementBuffer().first);
  }

  void GroupsContextMenu::load() {
    mw->loadGroup(element);
  }

  void GroupsContextMenu::add() {
    mw->addGroup(new Group("Group"), element);
  }

  ObjectsContextMenu::ObjectsContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Object*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    QMenu *menu = new BodiesContextMenu(element, "Add body");
    addMenu(menu);
  }

  void ObjectsContextMenu::paste() {
    mw->loadObject(element, mw->getElementBuffer().first);
  }

  void ObjectsContextMenu::load() {
    mw->loadObject(element);
  }

  BodiesContextMenu::BodiesContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Add rigid body", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addRigidBody()));
    addAction(action);
    action = new QAction("Add flexible body ffr", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addFlexibleBodyFFR()));
    addAction(action);
  }

  void BodiesContextMenu::addRigidBody() {
    mw->addObject(new RigidBody("RigidBody"), element);
  }

  void BodiesContextMenu::addFlexibleBodyFFR() {
    mw->addObject(new FlexibleBodyFFR("FlexibleBodyFFR"), element);
  }

  LinksContextMenu::LinksContextMenu(Element *element_, const QString &title,  QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Link*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    action = new QAction("Add kinetic excitation", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addKineticExcitation()));
    addAction(action);
    action = new QAction("Add spring damper", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addSpringDamper()));
    addAction(action);
    action = new QAction("Add directional spring damper", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addDirectionalSpringDamper()));
    addAction(action);
    action = new QAction("Add joint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addJoint()));
    addAction(action);
    action = new QAction("Add elastic joint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addElasticJoint()));
    addAction(action);
    action = new QAction("Add contact", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addContact()));
    addAction(action);
    QMenu *menu = new SignalsContextMenu(element, "Add signal");
    addMenu(menu);
    action = new QAction("Add generalized spring damper", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedSpringDamper()));
    addAction(action);
    action = new QAction("Add generalized friction", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedFriction()));
    addAction(action);
    action = new QAction("Add gear", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedGear()));
    addAction(action);
    action = new QAction("Add generalized elastic connection", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedElasticConnection()));
    addAction(action);
  }

  void LinksContextMenu::paste() {
    mw->loadLink(element, mw->getElementBuffer().first);
  }

  void LinksContextMenu::load() {
    mw->loadLink(element);
  }

  void LinksContextMenu::addKineticExcitation() {
    mw->addLink(new KineticExcitation("KineticExcitation"), element);
  }

  void LinksContextMenu::addSpringDamper() {
    mw->addLink(new SpringDamper("SpringDamper"), element);
  }

  void LinksContextMenu::addDirectionalSpringDamper() {
    mw->addLink(new DirectionalSpringDamper("DirectionalSpringDamper"), element);
  }

  void LinksContextMenu::addJoint() {
    mw->addLink(new Joint("Joint"), element);
  }

  void LinksContextMenu::addElasticJoint() {
    mw->addLink(new ElasticJoint("ElasticJoint"), element);
  }

  void LinksContextMenu::addContact() {
    mw->addLink(new Contact("Contact"), element);
  }

  void LinksContextMenu::addSignal() {
    SignalsContextMenu menu(element);
    menu.exec(QCursor::pos());
  }

  void LinksContextMenu::addGeneralizedSpringDamper() {
    mw->addLink(new GeneralizedSpringDamper("GeneralizedSpringDamper"), element);
  }

  void LinksContextMenu::addGeneralizedFriction() {
    mw->addLink(new GeneralizedFriction("GeneralizedFriction"), element);
  }

  void LinksContextMenu::addGeneralizedGear() {
    mw->addLink(new GeneralizedGear("GeneralizedGear"), element);
  }

  void LinksContextMenu::addGeneralizedElasticConnection() {
    mw->addLink(new GeneralizedElasticConnection("GeneralizedElasticConnection"), element);
  }

  ConstraintsContextMenu::ConstraintsContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Constraint*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    action = new QAction("Add generalized position constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedPositionConstraint()));
    addAction(action);
    action = new QAction("Add generalized velocity constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedVelocityConstraint()));
    addAction(action);
    action = new QAction("Add generalized acceleration constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedAccelerationConstraint()));
    addAction(action);
    action = new QAction("Add gear constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedGearConstraint()));
    addAction(action);
    action = new QAction("Add joint constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addJointConstraint()));
    addAction(action);
    action = new QAction("Add generalized connection constraint", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedConnectionConstraint()));
    addAction(action);
  }

  void ConstraintsContextMenu::paste() {
    mw->loadConstraint(element, mw->getElementBuffer().first);
  }

  void ConstraintsContextMenu::load() {
    mw->loadConstraint(element);
  }

  void ConstraintsContextMenu::addGeneralizedGearConstraint() {
    mw->addConstraint(new GeneralizedGearConstraint("GeneralizedGearConstraint"), element);
  }

  void ConstraintsContextMenu::addGeneralizedPositionConstraint() {
    mw->addConstraint(new GeneralizedPositionConstraint("GeneralizedPositionConstraint"), element);
  }

  void ConstraintsContextMenu::addGeneralizedVelocityConstraint() {
    mw->addConstraint(new GeneralizedVelocityConstraint("GeneralizedVelocityConstraint"), element);
  }

  void ConstraintsContextMenu::addGeneralizedAccelerationConstraint() {
    mw->addConstraint(new GeneralizedAccelerationConstraint("GeneralizedAccelerationConstraint"), element);
  }

  void ConstraintsContextMenu::addJointConstraint() {
    mw->addConstraint(new JointConstraint("JointConstraint"), element);
  }

  void ConstraintsContextMenu::addGeneralizedConnectionConstraint() {
    mw->addConstraint(new GeneralizedConnectionConstraint("GeneralizedConnectionConstraint"), element);
  }

  ObserversContextMenu::ObserversContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Paste", this);
    action->setEnabled(dynamic_cast<Observer*>(mw->getElementBuffer().first));
    connect(action,SIGNAL(triggered()),this,SLOT(paste()));
    addAction(action);
    action = new QAction("Load", this);
    connect(action,SIGNAL(triggered()),this,SLOT(load()));
    addAction(action);
    addSeparator();
    action = new QAction("Add mechanical link observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addMechanicalLinkObserver()));
    addAction(action);
    action = new QAction("Add mechanical constraint observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addMechanicalConstraintObserver()));
    addAction(action);
    action = new QAction("Add contact observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addContactObserver()));
    addAction(action);
    action = new QAction("Add frame observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addFrameObserver()));
    addAction(action);
    action = new QAction("Add rigid body observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addRigidBodyObserver()));
    addAction(action);
    action = new QAction("Add kinematic coordinates observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addKinematicCoordinatesObserver()));
    addAction(action);
    action = new QAction("Add relative kinematics observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addRelativeKinematicsObserver()));
    addAction(action);
    action = new QAction("Add rigid body system observer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addRigidBodySystemObserver()));
    addAction(action);
  }

  void ObserversContextMenu::paste() {
    mw->loadObserver(element, mw->getElementBuffer().first);
  }

  void ObserversContextMenu::load() {
    mw->loadObserver(element);
  }

  void ObserversContextMenu::addMechanicalLinkObserver() {
    mw->addObserver(new MechanicalLinkObserver("MechanicalLinkObserver"), element);
  }

  void ObserversContextMenu::addMechanicalConstraintObserver() {
    mw->addObserver(new MechanicalConstraintObserver("MechanicalConstraintObserver"), element);
  }

  void ObserversContextMenu::addContactObserver() {
    mw->addObserver(new ContactObserver("ContactObserver"), element);
  }

  void ObserversContextMenu::addFrameObserver() {
    mw->addObserver(new FrameObserver("FrameObserver"), element);
  }

  void ObserversContextMenu::addRigidBodyObserver() {
    mw->addObserver(new RigidBodyObserver("RigidBodyObserver"), element);
  }

  void ObserversContextMenu::addKinematicCoordinatesObserver() {
    mw->addObserver(new KinematicCoordinatesObserver("KinematicCoordinatesObserver"), element);
  }

  void ObserversContextMenu::addRelativeKinematicsObserver() {
    mw->addObserver(new RelativeKinematicsObserver("RelativeKinematicsObserver"), element);
  }

  void ObserversContextMenu::addRigidBodySystemObserver() {
    mw->addObserver(new RigidBodySystemObserver("RigidBodySystemObserver"), element);
  }

  SignalsContextMenu::SignalsContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QMenu *menu = new SensorsContextMenu(element,"Add sensor");
    addMenu(menu);
    QAction *action = new QAction("Add multiplexer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addMultiplexer()));
    addAction(action);
    action = new QAction("Add demultiplexer", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addDemultiplexer()));
    addAction(action);
    action = new QAction("Add linear transfer system", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addLinearTransferSystem()));
    addAction(action);
    action = new QAction("Add signal operation", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addSignalOperation()));
    addAction(action);
    action = new QAction("Add extern signal source", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addExternSignalSource()));
    addAction(action);
    action = new QAction("Add extern signal sink", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addExternSignalSink()));
    addAction(action);
  }

  void SignalsContextMenu::addSensor() {
    SensorsContextMenu menu(element);
    menu.exec(QCursor::pos());
  }

  void SignalsContextMenu::addMultiplexer() {
    mw->addLink(new Multiplexer("Multiplexer"), element);
  }

  void SignalsContextMenu::addDemultiplexer() {
    mw->addLink(new Demultiplexer("Demultiplexer"), element);
  }

  void SignalsContextMenu::addLinearTransferSystem() {
    mw->addLink(new LinearTransferSystem("LinearTransferSystem"), element);
  }

  void SignalsContextMenu::addSignalOperation() {
    mw->addLink(new SignalOperation("SignalOperation"), element);
  }

  void SignalsContextMenu::addExternSignalSource() {
    mw->addLink(new ExternSignalSource("ExternSignalSource"), element);
  }

  void SignalsContextMenu::addExternSignalSink() {
    mw->addLink(new ExternSignalSink("ExternSignalSink"), element);
  }

  SensorsContextMenu::SensorsContextMenu(Element *element_, const QString &title, QWidget *parent) : QMenu(title,parent), element(element_) {
    QAction *action = new QAction("Add generalized position sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedPositionSensor()));
    addAction(action);
    action = new QAction("Add generalized velocity sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addGeneralizedVelocitySensor()));
    addAction(action);
    action = new QAction("Add position sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addPositionSensor()));
    addAction(action);
    action = new QAction("Add orientation sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addOrientationSensor()));
    addAction(action);
    action = new QAction("Add velocity sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addVelocitySensor()));
    addAction(action);
    action = new QAction("Add angular velocity sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addAngularVelocitySensor()));
    addAction(action);
    action = new QAction("Add function sensor", this);
    connect(action,SIGNAL(triggered()),this,SLOT(addFunctionSensor()));
    addAction(action);
  }

  void SensorsContextMenu::addGeneralizedPositionSensor() {
    mw->addLink(new GeneralizedPositionSensor("GeneralizedPositionSensor"), element);
  }

  void SensorsContextMenu::addGeneralizedVelocitySensor() {
    mw->addLink(new GeneralizedVelocitySensor("GeneralizedVelocitySensor"), element);
  }

  void SensorsContextMenu::addPositionSensor() {
    mw->addLink(new PositionSensor("PositionSensor"), element);
  }

  void SensorsContextMenu::addOrientationSensor() {
    mw->addLink(new OrientationSensor("OrientationSensor"), element);
  }

  void SensorsContextMenu::addVelocitySensor() {
    mw->addLink(new VelocitySensor("VelocitySensor"), element);
  }

  void SensorsContextMenu::addAngularVelocitySensor() {
    mw->addLink(new AngularVelocitySensor("AngularVelocitySensor"), element);
  }

  void SensorsContextMenu::addFunctionSensor() {
    mw->addLink(new FunctionSensor("FunctionSensor"), element);
  }

}
