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

#ifndef _SOLVER_PROPERTY_DIALOG_H_
#define _SOLVER_PROPERTY_DIALOG_H_

#include "property_dialog.h"
#include "widget.h"

namespace MBSimGUI {

  class Solver;
  class Integrator;
  class DOPRI5Integrator;
  class RADAU5Integrator;
  class LSODEIntegrator;
  class LSODARIntegrator;
  class TimeSteppingIntegrator;
  class EulerExplicitIntegrator;
  class RKSuiteIntegrator;
  class Eigenanalyser;
  class VecWidget;
  class ExtWidget;

  class ToleranceWidgetFactory : public WidgetFactory {
    public:
      ToleranceWidgetFactory(const QString &type="");
      QWidget* createWidget(int i=0);
      QString getName(int i=0) const { return name[i]; }
      MBXMLUtils::FQN getXMLName(int i=0) const { return xmlName[i]; }
      int getSize() const { return name.size(); }
    protected:
      std::vector<QString> name;
      std::vector<MBXMLUtils::FQN> xmlName;
      QString type;
  };

  class SolverPropertyDialog : public PropertyDialog {

    public:
      SolverPropertyDialog(Solver *solver, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
      virtual void toWidget(Solver *solver);
      virtual void fromWidget(Solver *solver);
      void toWidget() {toWidget(solver);}
      void fromWidget() {fromWidget(solver);}
      Solver* getSolver() {return solver;}
    protected:
      Solver *solver;
      ExtWidget *embed;
  };

  class IntegratorPropertyDialog : public SolverPropertyDialog {

    public:
      IntegratorPropertyDialog(Integrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      VecWidget *z0;
      ExtWidget *startTime, *endTime, *plotStepSize, *initialState;
  };

  class DOPRI5IntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      DOPRI5IntegratorPropertyDialog(DOPRI5Integrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *absTol, *relTol, *initialStepSize, *maximalStepSize, *maxSteps;
  };

  class RADAU5IntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      RADAU5IntegratorPropertyDialog(RADAU5Integrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *absTol, *relTol, *initialStepSize, *maximalStepSize, *maxSteps;
  };

  class LSODEIntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      LSODEIntegratorPropertyDialog(LSODEIntegrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *absTol, *relTol, *initialStepSize, *maximalStepSize, *minimalStepSize, *maxSteps, *stiff;
  };

  class LSODARIntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      LSODARIntegratorPropertyDialog(LSODARIntegrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *absTol, *relTol, *initialStepSize, *maximalStepSize, *minimalStepSize, *plotOnRoot, *gMax, *gdMax;
  };

  class TimeSteppingIntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      TimeSteppingIntegratorPropertyDialog(TimeSteppingIntegrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *stepSize;
  };

  class EulerExplicitIntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      EulerExplicitIntegratorPropertyDialog(EulerExplicitIntegrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *stepSize;
  };

  class RKSuiteIntegratorPropertyDialog : public IntegratorPropertyDialog {

    public:
      RKSuiteIntegratorPropertyDialog(RKSuiteIntegrator *integrator, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *method, *relTol, *threshold, *initialStepSize;
  };

  class EigenanalyserPropertyDialog : public SolverPropertyDialog {

    public:
      EigenanalyserPropertyDialog(Eigenanalyser *eigenanalyser, QWidget * parent = 0, Qt::WindowFlags f = 0);
      xercesc::DOMElement* initializeUsingXML(xercesc::DOMElement *element);
      xercesc::DOMElement* writeXMLFile(xercesc::DOMNode *element, xercesc::DOMNode *ref=NULL);
    protected:
      ExtWidget *startTime, *endTime, *plotStepSize, *initialState, *task, *amplitude, *mode, *determineEquilibriumState;
  };

}

#endif
