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
#include "integrator.h"
#include "analyser.h"
#include "solver_view.h"
#include "solver_property_dialog.h"
#include "utils.h"
#include "mainwindow.h"
#include <QEvent>

namespace MBSimGUI {

  extern MainWindow *mw;

  SolverViewContextMenu::SolverViewContextMenu(QWidget *parent) : QMenu(parent) {
    QActionGroup *actionGroup = new QActionGroup(this);
    QAction *action = new QAction(Utils::QIconCached("newobject.svg"),"DOPRI5 integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"RADAU5 integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"LSODE integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"LSODAR integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"Time stepping integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"Euler explicit integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"RKSuite integrator", this);
    actionGroup->addAction(action);
    action = new QAction(Utils::QIconCached("newobject.svg"),"Eigenanalyser", this);
    actionGroup->addAction(action);
    addActions(actionGroup->actions());
    connect(actionGroup,SIGNAL(triggered(QAction*)),this,SLOT(selectSolver(QAction*)));
  }

  void SolverViewContextMenu::selectSolver(QAction *action) {
    QActionGroup *actionGroup = action->actionGroup();
    QList<QAction*> list = actionGroup->actions();
    mw->selectSolver(list.indexOf(action));
  }

  SolverView::SolverView() : i(0) {
    solver.push_back(new DOPRI5Integrator);
    solver.push_back(new RADAU5Integrator);
    solver.push_back(new LSODEIntegrator);
    solver.push_back(new LSODARIntegrator);
    solver.push_back(new TimeSteppingIntegrator);
    solver.push_back(new EulerExplicitIntegrator);
    solver.push_back(new RKSuiteIntegrator);
    solver.push_back(new Eigenanalyser);
    type.push_back("DOPRI5 integrator");
    type.push_back("RADAU5 integrator");
    type.push_back("LSODE integrator");
    type.push_back("LSODAR integrator");
    type.push_back("Time stepping integrator");
    type.push_back("Euler explicit integrator");
    type.push_back("RKSuite integrator");
    type.push_back("Eigenanalyser");
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,SIGNAL(customContextMenuRequested(const QPoint&)),this,SLOT(openContextMenu()));

    installEventFilter(new IntegratorMouseEvent(this));
    setReadOnly(true);
  }

  SolverView::~SolverView() {
    for(size_t i=0; i<solver.size(); i++)
      delete solver[i];
  }

  void SolverView::openContextMenu() {
    QMenu *menu = createContextMenu();
    menu->exec(QCursor::pos());
    delete menu;
  }

  void SolverView::setSolver(Solver *solver_) {
    if(dynamic_cast<DOPRI5Integrator*>(solver_))
      i=0;
    else if(dynamic_cast<RADAU5Integrator*>(solver_))
      i=1;
    else if(dynamic_cast<LSODEIntegrator*>(solver_))
      i=2;
    else if(dynamic_cast<LSODARIntegrator*>(solver_))
      i=3;
    else if(dynamic_cast<TimeSteppingIntegrator*>(solver_))
      i=4;
    else if(dynamic_cast<EulerExplicitIntegrator*>(solver_))
      i=5;
    else if(dynamic_cast<RKSuiteIntegrator*>(solver_))
      i=6;
    else if(dynamic_cast<Eigenanalyser*>(solver_))
      i=7;
    delete solver[i];
    solver[i] = solver_;
    updateText();
  }

  bool IntegratorMouseEvent::eventFilter(QObject *obj, QEvent *event) {
    if(event->type() == QEvent::MouseButtonDblClick) {
      mw->setAllowUndo(false);
      mw->updateParameters(view->getSolver());
      editor = view->getSolver()->createPropertyDialog();
      editor->setAttribute(Qt::WA_DeleteOnClose);
      editor->toWidget();
      editor->show();
      connect(editor,SIGNAL(apply()),this,SLOT(apply()));
      connect(editor,SIGNAL(finished(int)),this,SLOT(dialogFinished(int)));
      return true;
    }
    else if(event->type() == QEvent::MouseButtonPress) {
      mw->solverViewClicked();
      return true;
    }
    else
      return QObject::eventFilter(obj, event);
  }

  void IntegratorMouseEvent::dialogFinished(int result) {
    if(result != 0)
      mw->setProjectChanged(true);
    editor->fromWidget();
    editor = 0;
    mw->setAllowUndo(true);
  }

  void IntegratorMouseEvent::apply() {
    mw->setProjectChanged(true);
    editor->fromWidget();
  }

}
