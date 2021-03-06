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
#include "dialogs.h"
#include "frame.h"
#include "contour.h"
#include "group.h"
#include "rigid_body.h"
#include "signal_.h"
#include "constraint.h"
#include "variable_widgets.h"
#include "mainwindow.h"
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QTreeWidget>
#include <QTableWidget>
#include <QLabel>
#include <QComboBox>
#include <QSpinBox>

using namespace std;

namespace MBSimGUI {

  extern MainWindow *mw;

  //EvalDialog::EvalDialog(VariableWidget *var_) : var(var_) {
  EvalDialog::EvalDialog(const vector<vector<QString> > &var_) {
    var.resize(var_.size());
    for(int i=0; i<var_.size(); i++) {
      var[i].resize(var_[i].size());
      for(int j=0; j<var[i].size(); j++)
        var[i][j] = var_[i][j].toDouble();
    }

    QVBoxLayout *mainlayout = new QVBoxLayout;
    setLayout(mainlayout);

    QGridLayout *layout = new QGridLayout;
    mainlayout->addLayout(layout);

    layout->addWidget(new QLabel("Format:"),0,0);
    format = new QComboBox;
    format->addItems(QStringList() << "e" << "E" << "f" << "g" << "G");
    format->setCurrentIndex(3);
    layout->addWidget(format,0,1);
    connect(format, SIGNAL(currentIndexChanged(int)), this, SLOT(updateWidget()));

    layout->addWidget(new QLabel("Precision:"),0,2);
    precision = new QSpinBox;
    precision->setValue(6);
    layout->addWidget(precision,0,3);
    connect(precision, SIGNAL(valueChanged(int)), this, SLOT(updateWidget()));

    tab = new QTableWidget;
    tab->setRowCount(var.size());
    tab->setColumnCount(var.size()?var[0].size():0);
    for(int i=0; i<tab->rowCount(); i++) {
      for(int j=0; j<tab->columnCount(); j++)
        tab->setItem(i,j,new QTableWidgetItem(QString::number(var[i][j],'g',6)));
    }

    layout->addWidget(tab,1,0,1,5);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(Qt::Horizontal);
    buttonBox->addButton(QDialogButtonBox::Close);
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    mainlayout->addWidget(buttonBox);

    layout->setColumnStretch(4, 10);
//    layout->setColumnStretch(2, 20);

    setWindowTitle("Expression evaluation");
  }

  void EvalDialog::updateWidget() {
    QString f = format->currentText();
    int p = precision->value();
    for(int i=0; i<tab->rowCount(); i++) {
      for(int j=0; j<tab->columnCount(); j++)
        tab->item(i,j)->setText(QString::number(var[i][j],f[0].toAscii(),p));
    }
  }

  RigidBodyBrowser::RigidBodyBrowser(Element* element_, RigidBody* rigidBody, QWidget *parentObject_) : QDialog(parentObject_), selection(rigidBody), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    rigidBodyList = new QTreeWidget;
    rigidBodyList->setColumnCount(1);
    mainLayout->addWidget(rigidBodyList,0,0);
    connect(rigidBodyList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForRigidBody(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("RigidBody browser");
  }

  void RigidBodyBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(rigidBodyList->currentItem())
      checkForRigidBody(rigidBodyList->currentItem(),0);
  }

  void RigidBodyBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void RigidBodyBrowser::updateWidget(RigidBody *sel) {
    selection = sel;
    rigidBodyList->clear();
    savedItem = 0;
    mbs2RigidBodyTree(element,rigidBodyList->invisibleRootItem());
    rigidBodyList->setCurrentItem(savedItem);
  }

  void RigidBodyBrowser::mbs2RigidBodyTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<RigidBody*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2RigidBodyTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfObjects(); i++)
        mbs2RigidBodyTree(ele->getObject(i),item);
    }
  }

  void RigidBodyBrowser::checkForRigidBody(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<RigidBody*>(item->getElement())) {
      mw->highlightObject(static_cast<RigidBody*>(item->getElement())->getID());
      okButton->setDisabled(false);
    }
    else
      okButton->setDisabled(true);
  }

  ObjectBrowser::ObjectBrowser(Element* element_, Object* object, QWidget *parentObject_) : QDialog(parentObject_), selection(object), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    objectList = new QTreeWidget;
    objectList->setColumnCount(1);
    mainLayout->addWidget(objectList,0,0);
    connect(objectList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForObject(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Object browser");
  }

  void ObjectBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(objectList->currentItem())
      checkForObject(objectList->currentItem(),0);
  }

  void ObjectBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void ObjectBrowser::updateWidget(Object *sel) {
    selection = sel;
    objectList->clear();
    savedItem = 0;
    mbs2ObjectTree(element,objectList->invisibleRootItem());
    objectList->setCurrentItem(savedItem);
  }

  void ObjectBrowser::mbs2ObjectTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Object*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2ObjectTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfObjects(); i++)
        mbs2ObjectTree(ele->getObject(i),item);
    }
  }

  void ObjectBrowser::checkForObject(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Object*>(item->getElement())) {
      mw->highlightObject(static_cast<Object*>(item->getElement())->getID());
      okButton->setDisabled(false);
    }
    else
      okButton->setDisabled(true);
  }

  LinkBrowser::LinkBrowser(Element* element_, Link* link, QWidget *parentLink_) : QDialog(parentLink_), selection(link), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    linkList = new QTreeWidget;
    linkList->setColumnCount(1);
    mainLayout->addWidget(linkList,0,0);
    connect(linkList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForLink(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Link browser");
  }

  void LinkBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(linkList->currentItem())
      checkForLink(linkList->currentItem(),0);
  }

  void LinkBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void LinkBrowser::updateWidget(Link *sel) {
    selection = sel;
    linkList->clear();
    savedItem = 0;
    mbs2LinkTree(element,linkList->invisibleRootItem());
    linkList->setCurrentItem(savedItem);
  }

  void LinkBrowser::mbs2LinkTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Link*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2LinkTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfLinks(); i++)
        mbs2LinkTree(ele->getLink(i),item);
    }
  }

  void LinkBrowser::checkForLink(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Link*>(item->getElement())) {
      mw->highlightObject(static_cast<Link*>(item->getElement())->getID());
      okButton->setDisabled(false);
    }
    else
      okButton->setDisabled(true);
  }

  ConstraintBrowser::ConstraintBrowser(Element* element_, Constraint* constraint, QWidget *parentConstraint_) : QDialog(parentConstraint_), selection(constraint), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    constraintList = new QTreeWidget;
    constraintList->setColumnCount(1);
    mainLayout->addWidget(constraintList,0,0);
    connect(constraintList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForConstraint(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Constraint browser");
  }

  void ConstraintBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(constraintList->currentItem())
      checkForConstraint(constraintList->currentItem(),0);
  }

  void ConstraintBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void ConstraintBrowser::updateWidget(Constraint *sel) {
    selection = sel;
    constraintList->clear();
    savedItem = 0;
    mbs2ConstraintTree(element,constraintList->invisibleRootItem());
    constraintList->setCurrentItem(savedItem);
  }

  void ConstraintBrowser::mbs2ConstraintTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Constraint*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2ConstraintTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfConstraints(); i++)
        mbs2ConstraintTree(ele->getConstraint(i),item);
    }
  }

  void ConstraintBrowser::checkForConstraint(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Constraint*>(item->getElement())) {
      mw->highlightObject(static_cast<Constraint*>(item->getElement())->getID());
      okButton->setDisabled(false);
    }
    else
      okButton->setDisabled(true);
  }

  FrameBrowser::FrameBrowser(Element* element_, Frame* frame, QWidget *parentObject_) : QDialog(parentObject_), selection(frame), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    frameList = new QTreeWidget;
    frameList->setColumnCount(1);
    mainLayout->addWidget(frameList,0,0);
    connect(frameList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForFrame(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Frame browser");
  }

  void FrameBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(frameList->currentItem())
      checkForFrame(frameList->currentItem(),0);
  }

  void FrameBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void FrameBrowser::updateWidget(Frame *sel) {
    selection = sel;
    frameList->clear();
    savedItem = 0;
    mbs2FrameTree(element,frameList->invisibleRootItem());
    frameList->setCurrentItem(savedItem);
  }

  void FrameBrowser::mbs2FrameTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Object*>(ele) || dynamic_cast<Frame*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfFrames(); i++)
        mbs2FrameTree(ele->getFrame(i),item);
      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2FrameTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfObjects(); i++)
        mbs2FrameTree(ele->getObject(i),item);
    }
  }

  void FrameBrowser::checkForFrame(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Frame*>(item->getElement())) {
      mw->highlightObject(static_cast<Frame*>(item->getElement())->getID());
      okButton->setDisabled(false);
    } else
      okButton->setDisabled(true);
  }

  ContourBrowser::ContourBrowser(Element* element_, Contour* contour, QWidget *parentObject_) : QDialog(parentObject_), selection(contour), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    contourList = new QTreeWidget;
    contourList->setColumnCount(1);
    mainLayout->addWidget(contourList,0,0);
    connect(contourList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForContour(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Contour browser");
  }

  void ContourBrowser::showEvent(QShowEvent *event) {
    QDialog::showEvent(event);
    oldID = mw->getHighlightedObject();
    if(contourList->currentItem())
      checkForContour(contourList->currentItem(),0);
  }

  void ContourBrowser::hideEvent(QHideEvent *event) {
    QDialog::hideEvent(event);
    mw->highlightObject(oldID);
  }

  void ContourBrowser::updateWidget(Contour *sel) {
    selection = sel;
    contourList->clear();
    savedItem = 0;
    mbs2ContourTree(element,contourList->invisibleRootItem());
    contourList->setCurrentItem(savedItem);
  }

  void ContourBrowser::mbs2ContourTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Object*>(ele)  || dynamic_cast<Contour*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfContours(); i++)
        mbs2ContourTree(ele->getContour(i),item);
      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2ContourTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfObjects(); i++)
        mbs2ContourTree(ele->getObject(i),item);
    }
  }

  void ContourBrowser::checkForContour(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Contour*>(item->getElement())) {
      mw->highlightObject(static_cast<Contour*>(item->getElement())->getID());
      okButton->setDisabled(false);
    }
    else
      okButton->setDisabled(true);
  }

  SignalBrowser::SignalBrowser(Element* element_, Signal* signal, QWidget *parentSignal_) : QDialog(parentSignal_), selection(signal), savedItem(0), element(element_) {
    QGridLayout* mainLayout=new QGridLayout;
    setLayout(mainLayout);
    signalList = new QTreeWidget;
    signalList->setColumnCount(1);
    mainLayout->addWidget(signalList,0,0);
    connect(signalList, SIGNAL(itemClicked(QTreeWidgetItem*, int)), this, SLOT(checkForSignal(QTreeWidgetItem*,int)));

    okButton = new QPushButton("Ok");
    if(!selection)
      okButton->setDisabled(true);
    mainLayout->addWidget(okButton,1,0);
    connect(okButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    QPushButton *button = new QPushButton("Cancel");
    mainLayout->addWidget(button,1,1);
    connect(button, SIGNAL(clicked(bool)), this, SLOT(reject()));

    setWindowTitle("Signal browser");
  }

  void SignalBrowser::updateWidget(Signal *sel) {
    selection = sel;
    signalList->clear();
    savedItem = 0;
    mbs2SignalTree(element,signalList->invisibleRootItem());
    signalList->setCurrentItem(savedItem);
  }

  void SignalBrowser::mbs2SignalTree(Element* ele, QTreeWidgetItem* parentItem) {
    if(dynamic_cast<Group*>(ele) || dynamic_cast<Signal*>(ele)) {

      ElementItem *item = new ElementItem(ele);
      item->setText(0,ele->getName());

      if(ele == selection)
        savedItem = item;

      parentItem->addChild(item);

      for(int i=0; i<ele->getNumberOfGroups(); i++)
        mbs2SignalTree(ele->getGroup(i),item);
      for(int i=0; i<ele->getNumberOfLinks(); i++)
        mbs2SignalTree(ele->getLink(i),item);
    }
  }

  void SignalBrowser::checkForSignal(QTreeWidgetItem* item_,int) {
    ElementItem* item = static_cast<ElementItem*>(item_);
    if(dynamic_cast<Signal*>(item->getElement()))
      okButton->setDisabled(false);
    else
      okButton->setDisabled(true);
  }

}
