/* Copyright (C) 2004-2009 MBSim Development Team
 *
 * This library is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU Lesser General Public 
 * License as published by the Free Software Foundation; either 
 * version 2.1 of the License, or (at your option) any later version. 
 *  
 * This library is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * Lesser General Public License for more details. 
 *  
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library; if not, write to the Free Software 
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301 USA
 *
 * Contact: martin.o.foerg@googlemail.com
 *          rzander@users.berlios.de
 */

#include<config.h>
#include<unistd.h>
#include "mbsim/dynamic_system_solver.h"
#include "mbsim/modelling_interface.h"
#include "mbsim/frame.h"
#include "mbsim/contour.h"
#include "mbsim/link.h"
#include "mbsim/graph.h"
#include "mbsim/object.h"
#include "mbsim/observer.h"
#include "mbsim/integrators/integrator.h"
#include "mbsim/utils/eps.h"
#include "dirent.h"
#include <mbsim/environment.h>
#include <mbsim/objectfactory.h>
#include <mbsim/xmlnamespacemapping.h>

#include <H5Cpp.h>
#include <hdf5serie/fileserie.h>
#include <hdf5serie/simpleattribute.h>
#include <hdf5serie/simpledataset.h>
#include <unistd.h>
#include <limits>

#ifdef HAVE_ANSICSIGNAL
#  include <signal.h>
#endif

#ifdef HAVE_OPENMBVCPPINTERFACE
#include "openmbvcppinterface/group.h"
#endif

//#ifdef _OPENMP
//#include <omp.h>
//#endif

using namespace std;
using namespace MBXMLUtils;
using namespace fmatvec;

namespace MBSim {
  double tP = 20.0;
  bool gflag = false;

  bool DynamicSystemSolver::exitRequest = false;

  DynamicSystemSolver::DynamicSystemSolver() :
      Group("Default"), maxIter(10000), highIter(1000), maxDampingSteps(3), lmParm(0.001), contactSolver(FixedPointSingle), impactSolver(FixedPointSingle), strategy(local), linAlg(LUDecomposition), stopIfNoConvergence(false), dropContactInfo(false), useOldla(true), numJac(false), checkGSize(true), limitGSize(500), warnLevel(0), peds(false), driftCount(1), flushEvery(100000), flushCount(flushEvery), reorganizeHierarchy(true), tolProj(1e-15), alwaysConsiderContact(true), inverseKinetics(false), rootID(0), gTol(1e-8), gdTol(1e-10), gddTol(1e-12), laTol(1e-12), LaTol(1e-10), INFO(true), READZ0(false), truncateSimulationFiles(true) {
    constructor();
  }

  DynamicSystemSolver::DynamicSystemSolver(const string &projectName) :
      Group(projectName), maxIter(10000), highIter(1000), maxDampingSteps(3), lmParm(0.001), contactSolver(FixedPointSingle), impactSolver(FixedPointSingle), strategy(local), linAlg(LUDecomposition), stopIfNoConvergence(false), dropContactInfo(false), useOldla(true), numJac(false), checkGSize(true), limitGSize(500), warnLevel(0), peds(false), driftCount(1), flushEvery(100000), flushCount(flushEvery), reorganizeHierarchy(true), tolProj(1e-15), alwaysConsiderContact(true), inverseKinetics(false), rootID(0), gTol(1e-8), gdTol(1e-10), gddTol(1e-12), laTol(1e-12), LaTol(1e-10), INFO(true), READZ0(false), truncateSimulationFiles(true) {
    constructor();
  }

  DynamicSystemSolver::~DynamicSystemSolver() {
    closePlot();
    H5::FileSerie::deletePIDFiles();
  }

  void DynamicSystemSolver::initialize() {
#ifdef HAVE_ANSICSIGNAL
    signal(SIGINT, sigInterruptHandler);
    signal(SIGTERM, sigInterruptHandler);
    signal(SIGABRT, sigAbortHandler);
#endif
    for (int stage = 0; stage < MBSim::LASTINITSTAGE; stage++) {
      if (INFO)
        cout << "Initializing stage " << stage << "/" << LASTINITSTAGE - 1 << endl;
      init((InitStage) stage);
      if (INFO)
        cout << "Done initializing stage " << stage << "/" << LASTINITSTAGE - 1 << endl;
    }
  }

  void DynamicSystemSolver::init(InitStage stage) {
    if (stage == MBSim::reorganizeHierarchy && reorganizeHierarchy) {
      if (INFO)
        cout << name << " (special group) stage==preInit:" << endl;

      vector<Object*> objList;
      buildListOfObjects(objList, true);

      vector<Frame*> frmList;
      buildListOfFrames(frmList, true);

      vector<Contour*> cntList;
      buildListOfContours(cntList, true);

      vector<Link*> lnkList;
      buildListOfLinks(lnkList, true);

      vector<ModellingInterface*> modellList;
      buildListOfModels(modellList, true);

      if (modellList.size())
        do {
          modellList[0]->processModellList(modellList, objList, lnkList);
        } while (modellList.size());

      vector<Link*> iKlnkList;
      buildListOfInverseKineticsLinks(iKlnkList, true);

      vector<Observer*> obsrvList;
      buildListOfObservers(obsrvList, true);

      dynamicsystem.clear(); // delete old DynamicSystem list
      object.clear(); // delete old object list
      frame.clear(); // delete old frame list
      contour.clear(); // delete old contour list
      link.clear(); // delete old link list
      inverseKineticsLink.clear(); // delete old link list
      observer.clear(); // delete old link list

      /* rename system structure */
      if (INFO)
        cout << "object List:" << endl;
      for (unsigned int i = 0; i < objList.size(); i++) {
        stringstream str;
        str << objList[i]->getPath('/');
        if (INFO)
          cout << str.str() << endl;
        objList[i]->setName(str.str());
      }
      if (INFO)
        cout << "frame List:" << endl;
      for (unsigned int i = 0; i < frmList.size(); i++) {
        stringstream str;
        str << frmList[i]->getParent()->getPath('/') << "/" << frmList[i]->getName();
        if (INFO)
          cout << str.str() << endl;
        frmList[i]->setName(str.str());
        addFrame((FixedRelativeFrame*) frmList[i]);
      }
      if (INFO)
        cout << "contour List:" << endl;
      for (unsigned int i = 0; i < cntList.size(); i++) {
        stringstream str;
        str << cntList[i]->getParent()->getPath('/') << "/" << cntList[i]->getName();
        if (INFO)
          cout << str.str() << endl;
        cntList[i]->setName(str.str());
        addContour(cntList[i]);
      }
      if (INFO)
        cout << "link List:" << endl;
      for (unsigned int i = 0; i < lnkList.size(); i++) {
        stringstream str;
        str << lnkList[i]->getParent()->getPath('/') << "/" << lnkList[i]->getName();
        if (INFO)
          cout << str.str() << endl;
        lnkList[i]->setName(str.str());
        addLink(lnkList[i]);
      }
      if (INFO)
        cout << "inverse kinetics link List:" << endl;
      for (unsigned int i = 0; i < iKlnkList.size(); i++) {
        stringstream str;
        str << iKlnkList[i]->getParent()->getPath('/') << "/" << iKlnkList[i]->getName();
        if (INFO)
          cout << str.str() << endl;
        iKlnkList[i]->setName(str.str());
        addInverseKineticsLink(iKlnkList[i]);
      }
      if (INFO)
        cout << "observer List:" << endl;
      for (unsigned int i = 0; i < obsrvList.size(); i++) {
        stringstream str;
        str << obsrvList[i]->getParent()->getPath('/') << "/" << obsrvList[i]->getName();
        if (INFO)
          cout << str.str() << endl;
        obsrvList[i]->setName(str.str());
        addObserver(obsrvList[i]);
      }

      /* matrix of body dependencies */
      SqrMat A(objList.size(), INIT, 0.);
      for (unsigned int i = 0; i < objList.size(); i++) {

        vector<Object*> parentBody = objList[i]->getObjectsDependingOn();

        for (unsigned int h = 0; h < parentBody.size(); h++) {
          unsigned int j = 0;
          bool foundBody = false;
          for (unsigned int k = 0; k < objList.size(); k++, j++) {
            if (objList[k] == parentBody[h]) {
              foundBody = true;
              break;
            }
          }

          if (foundBody) {
            A(i, j) = 2; // 2 means predecessor
            A(j, i) = 1; // 1 means successor
          }
        }
      }
      // if(INFO) cout << "A = " << A << endl;

      vector<Graph*> bufGraph;
      int nt = 0;
      for (int i = 0; i < A.size(); i++) {
        double a = max(A.T().col(i));
        if (a > 0 && fabs(A(i, i) + 1) > epsroot()) { // root of relativ kinematics
          stringstream str;
          str << "InvisibleGraph" << nt++;
          Graph *graph = new Graph(str.str());
          addToGraph(graph, A, i, objList);
          graph->setPlotFeatureRecursive(plotRecursive, enabled); // the generated invisible graph must always walk through the plot functions
          bufGraph.push_back(graph);
        }
        else if (fabs(a) < epsroot()) // absolut kinematics
          addObject(objList[i]);
      }
      // if(INFO) cout << "A = " << A << endl;

      for (unsigned int i = 0; i < bufGraph.size(); i++) {
        addGroup(bufGraph[i]);
      }

      if (INFO)
        cout << "End of special group stage==preInit" << endl;

      // after reorganizing a resize is required
      init(resize);
      for (unsigned int i = 0; i < dynamicsystem.size(); i++)
        if (dynamic_cast<Graph*>(dynamicsystem[i]))
          static_cast<Graph*>(dynamicsystem[i])->co();
    }
    else if (stage == resize) {
      calcqSize();
      calcuSize(0);
      calcuSize(1);
      setqInd(0);
      setuInd(0, 0);
      setuInd(0, 1);
      sethSize(uSize[0], 0);
      sethSize(uSize[1], 1);
      sethInd(0, 0);
      sethInd(0, 1);
      setxInd(0);
      setUpLinks(); // is needed by calcgSize()

      calcxSize();

      calclaInverseKineticsSize();
      calcbInverseKineticsSize();

      calclaSize(0);
      calcgSize(0);
      calcgdSize(0);
      calcrFactorSize(0);
      calcsvSize();
      svSize += 1; // TODO additional event for drift 
      calcLinkStatusSize();
      calcLinkStatusRegSize();

      if (INFO)
        cout << "qSize = " << qSize << endl;
      if (INFO)
        cout << "uSize[0] = " << uSize[0] << endl;
      if (INFO)
        cout << "xSize = " << xSize << endl;
      if (INFO)
        cout << "gSize = " << gSize << endl;
      if (INFO)
        cout << "gdSize = " << gdSize << endl;
      if (INFO)
        cout << "laSize = " << laSize << endl;
      if (INFO)
        cout << "svSize = " << svSize << endl;
      if (INFO)
        cout << "LinkStatusSize = " << LinkStatusSize << endl;
      if (INFO)
        cout << "LinkStatusRegSize = " << LinkStatusRegSize << endl;
      if (INFO)
        cout << "hSize[0] = " << hSize[0] << endl;

      if (INFO)
        cout << "uSize[1] = " << uSize[1] << endl;
      if (INFO)
        cout << "hSize[1] = " << hSize[1] << endl;

      Group::init(stage);
    }
    else if (stage == unknownStage) {
      setDynamicSystemSolver(this);

      MParent[0].resize(getuSize(0));
      MParent[1].resize(getuSize(1));
      TParent.resize(getqSize(), getuSize());
      LLMParent[0].resize(getuSize(0));
      LLMParent[1].resize(getuSize(1));
      WParent[0].resize(getuSize(0), getlaSize());
      VParent[0].resize(getuSize(0), getlaSize());
      WParent[1].resize(getuSize(1), getlaSize());
      VParent[1].resize(getuSize(1), getlaSize());
      wbParent.resize(getlaSize());
      laParent.resize(getlaSize());
      rFactorParent.resize(getlaSize());
      sParent.resize(getlaSize());
      if (impactSolver == RootFinding)
        resParent.resize(getlaSize());
      gParent.resize(getgSize());
      gdParent.resize(getgdSize());
      zdParent.resize(getzSize());
      udParent1.resize(getuSize(1));
      hParent[0].resize(getuSize(0));
      hParent[1].resize(getuSize(1));
      rParent[0].resize(getuSize(0));
      rParent[1].resize(getuSize(1));
      fParent.resize(getxSize());
      svParent.resize(getsvSize());
      jsvParent.resize(getsvSize());
      LinkStatusParent.resize(getLinkStatusSize());
      LinkStatusRegParent.resize(getLinkStatusRegSize());
      WInverseKineticsParent[0].resize(hSize[0], laInverseKineticsSize);
      WInverseKineticsParent[1].resize(hSize[1], laInverseKineticsSize);
      bInverseKineticsParent.resize(bInverseKineticsSize, laInverseKineticsSize);
      laInverseKineticsParent.resize(laInverseKineticsSize);
      corrParent.resize(getgdSize());

      updateMRef(MParent[0], 0);
      updateMRef(MParent[1], 1);
      updateTRef(TParent);
      updateLLMRef(LLMParent[0], 0);
      updateLLMRef(LLMParent[1], 1);

      Group::init(stage);

      updatesvRef(svParent);
      updatejsvRef(jsvParent);
      updateLinkStatusRef(LinkStatusParent);
      updateLinkStatusRegRef(LinkStatusRegParent);
      updatezdRef(zdParent);
      updateudRef(udParent1, 1);
      updateudallRef(udParent1, 1);
      updatelaRef(laParent);
      updategRef(gParent);
      updategdRef(gdParent);
      updatehRef(hParent[0], 0);
      updaterRef(rParent[0], 0);
      updatehRef(hParent[1], 1);
      updaterRef(rParent[1], 1);
      updateWRef(WParent[0], 0);
      updateWRef(WParent[1], 1);
      updateVRef(VParent[0], 0);
      updateVRef(VParent[1], 1);
      updatewbRef(wbParent);

      updatelaInverseKineticsRef(laInverseKineticsParent);
      updateWInverseKineticsRef(WInverseKineticsParent[0], 0);
      updateWInverseKineticsRef(WInverseKineticsParent[1], 1);
      updatebInverseKineticsRef(bInverseKineticsParent);
      updatehRef(hParent[1], 1);
      updaterRef(rParent[1], 1);
      updateWRef(WParent[1], 1);
      updateVRef(VParent[1], 1);

      if (impactSolver == RootFinding)
        updateresRef(resParent);
      updaterFactorRef(rFactorParent);

      // contact solver specific settings
      if (INFO)
        cout << "  use contact solver \'" << getSolverInfo() << "\' for contact situations" << endl;
      if (contactSolver == GaussSeidel)
        solveConstraints_ = &DynamicSystemSolver::solveConstraintsGaussSeidel;
      else if (contactSolver == LinearEquations) {
        solveConstraints_ = &DynamicSystemSolver::solveConstraintsLinearEquations;
        cerr << "WARNING: solveLL is only valid for bilateral constrained systems!" << endl;
      }
      else if (contactSolver == FixedPointSingle)
        solveConstraints_ = &DynamicSystemSolver::solveConstraintsFixpointSingle;
      else if (contactSolver == RootFinding) {
        cerr << "WARNING: RootFinding solver is BUGGY at least if there is friction!" << endl;
        solveConstraints_ = &DynamicSystemSolver::solveConstraintsRootFinding;
      }
      else
        throw MBSimError("ERROR (DynamicSystemSolver::init()): Unknown contact solver");

      // impact solver specific settings
      if (INFO)
        cout << "  use impact solver \'" << getSolverInfo() << "\' for impact situations" << endl;
      if (impactSolver == GaussSeidel)
        solveImpacts_ = &DynamicSystemSolver::solveImpactsGaussSeidel;
      else if (impactSolver == LinearEquations) {
        solveImpacts_ = &DynamicSystemSolver::solveImpactsLinearEquations;
        cerr << "WARNING: solveLL is only valid for bilateral constrained systems!" << endl;
      }
      else if (impactSolver == FixedPointSingle)
        solveImpacts_ = &DynamicSystemSolver::solveImpactsFixpointSingle;
      else if (impactSolver == RootFinding) {
        cerr << "WARNING: RootFinding solver is BUGGY at least if there is friction!" << endl;
        solveImpacts_ = &DynamicSystemSolver::solveImpactsRootFinding;
      }
      else
        throw MBSimError("ERROR (DynamicSystemSolver::init()): Unknown impact solver");
    }
    else if (stage == MBSim::modelBuildup) {
      if (INFO)
        cout << "  initialising modelBuildup ..." << endl;
      Group::init(stage);
      setDynamicSystemSolver(this);
    }
    else if (stage == MBSim::preInit) {
      if (INFO)
        cout << "  initialising preInit ..." << endl;
      Group::init(stage);
      if (inverseKinetics)
        setUpInverseKinetics();
    }
    else if (stage == MBSim::plot) {
      if (INFO)
        cout << "  initialising plot-files ..." << endl;
      Group::init(stage);
#ifdef HAVE_OPENMBVCPPINTERFACE
      if (getPlotFeature(plotRecursive) == enabled)
        openMBVGrp->write(true, truncateSimulationFiles);
#endif
      if (INFO)
        cout << "...... done initialising." << endl << endl;
    }
    else if (stage == MBSim::calculateLocalInitialValues) {
      Group::initz();
      updateStateDependentVariables(0);
      updateg(0);
      Group::init(stage);
    }
    else
      Group::init(stage);
  }

  int DynamicSystemSolver::solveConstraintsFixpointSingle() {
    updaterFactors();

    checkConstraintsForTermination();
    if (term)
      return 0;

    int iter, level = 0;
    int checkTermLevel = 0;

    for (iter = 1; iter <= maxIter; iter++) {

      if (level < decreaseLevels.size() && iter > decreaseLevels(level)) {
        level++;
        decreaserFactors();
        cerr << endl << "WARNING: decreasing r-factors at iter = " << iter << endl;
        if (warnLevel >= 2)
          cerr << endl << "WARNING: decreasing r-factors at iter = " << iter << endl;
      }

      Group::solveConstraintsFixpointSingle();

      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkConstraintsForTermination();
        if (term)
          break;
      }
    }
    return iter;
  }

  int DynamicSystemSolver::solveImpactsFixpointSingle(double dt) {
    updaterFactors();

    checkImpactsForTermination(dt);
    if (term)
      return 0;

    int iter, level = 0;
    int checkTermLevel = 0;

    for (iter = 1; iter <= maxIter; iter++) {

      if (level < decreaseLevels.size() && iter > decreaseLevels(level)) {
        level++;
        decreaserFactors();
        cerr << endl << "WARNING: decreasing r-factors at iter = " << iter << endl;
        if (warnLevel >= 2)
          cerr << endl << "WARNING: decreasing r-factors at iter = " << iter << endl;
      }

      Group::solveImpactsFixpointSingle(dt);

      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkImpactsForTermination(dt);
        if (term)
          break;
      }
    }
    return iter;
  }

  int DynamicSystemSolver::solveConstraintsGaussSeidel() {
    checkConstraintsForTermination();
    if (term)
      return 0;

    int iter;
    int checkTermLevel = 0;

    for (iter = 1; iter <= maxIter; iter++) {
      Group::solveConstraintsGaussSeidel();
      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkConstraintsForTermination();
        if (term)
          break;
      }
    }
    return iter;
  }

  int DynamicSystemSolver::solveImpactsGaussSeidel(double dt) {
    checkImpactsForTermination(dt);
    if (term)
      return 0;

    int iter;
    int checkTermLevel = 0;

    for (iter = 1; iter <= maxIter; iter++) {
      Group::solveImpactsGaussSeidel(dt);
      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkImpactsForTermination(dt);
        if (term)
          break;
      }
    }
    return iter;
  }

  int DynamicSystemSolver::solveConstraintsRootFinding() {
    updaterFactors();

    int iter;
    int checkTermLevel = 0;

    updateresRef(resParent(0, laSize - 1));
    Group::solveConstraintsRootFinding();

    double nrmf0 = nrm2(res);
    Vec res0 = res.copy();

    checkConstraintsForTermination();
    if (term)
      return 0;

    DiagMat I(la.size(), INIT, 1);
    for (iter = 1; iter < maxIter; iter++) {

      if (Jprox.size() != la.size())
        Jprox.resize(la.size(), NONINIT);

      if (numJac) {
        for (int j = 0; j < la.size(); j++) {
          const double xj = la(j);
          double dx = epsroot() / 2.;
          
          do
            dx += dx;
          while (fabs(xj + dx - la(j)) < epsroot());

          la(j) += dx;
          Group::solveConstraintsRootFinding();
          la(j) = xj;
          Jprox.col(j) = (res - res0) / dx;
        }
      }
      else
        jacobianConstraints();
      Vec dx;
      if (linAlg == LUDecomposition)
        dx >> slvLU(Jprox, res0);
      else if (linAlg == LevenbergMarquardt) {
        SymMat J = SymMat(JTJ(Jprox) + lmParm * I);
        dx >> slvLL(J, Jprox.T() * res0);
      }
      else if (linAlg == PseudoInverse)
        dx >> slvLS(Jprox, res0);
      else
        throw MBSimError("Internal error");

      Vec La_old = la.copy();
      double alpha = 1;
      double nrmf = 1;
      for (int k = 0; k < maxDampingSteps; k++) {
        la = La_old - alpha * dx;
        Group::solveConstraintsRootFinding();
        nrmf = nrm2(res);
        if (nrmf < nrmf0)
          break;

        alpha *= .5;
      }
      nrmf0 = nrmf;
      res0 = res;

      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkConstraintsForTermination();
        if (term)
          break;
      }
    }
    return iter;
  }

  int DynamicSystemSolver::solveImpactsRootFinding(double dt) {
    updaterFactors();

    int iter;
    int checkTermLevel = 0;
    
    updateresRef(resParent(0, laSize - 1));
    Group::solveImpactsRootFinding(dt);

    double nrmf0 = nrm2(res);
    Vec res0 = res.copy();

    checkImpactsForTermination(dt);
    if (term)
      return 0;

    DiagMat I(la.size(), INIT, 1);
    for (iter = 1; iter < maxIter; iter++) {

      if (Jprox.size() != la.size())
        Jprox.resize(la.size(), NONINIT);

      if (numJac) {
        for (int j = 0; j < la.size(); j++) {
          const double xj = la(j);
          double dx = .5 * epsroot();
          
          do
            dx += dx;
          while (fabs(xj + dx - la(j)) < epsroot());
          
          la(j) += dx;
          Group::solveImpactsRootFinding(dt);
          la(j) = xj;
          Jprox.col(j) = (res - res0) / dx;
        }
      }
      else
        jacobianImpacts();
      Vec dx;
      if (linAlg == LUDecomposition)
        dx >> slvLU(Jprox, res0);
      else if (linAlg == LevenbergMarquardt) {
        SymMat J = SymMat(JTJ(Jprox) + lmParm * I);
        dx >> slvLL(J, Jprox.T() * res0);
      }
      else if (linAlg == PseudoInverse)
        dx >> slvLS(Jprox, res0);
      else
        throw MBSimError("Internal error");

      Vec La_old = la.copy();
      double alpha = 1.;
      double nrmf = 1;
      for (int k = 0; k < maxDampingSteps; k++) {
        la = La_old - alpha * dx;
        Group::solveImpactsRootFinding(dt);
        nrmf = nrm2(res);
        if (nrmf < nrmf0)
          break;
        alpha *= .5;
      }
      nrmf0 = nrmf;
      res0 = res;

      if (checkTermLevel >= checkTermLevels.size() || iter > checkTermLevels(checkTermLevel)) {
        checkTermLevel++;
        checkImpactsForTermination(dt);
        if (term)
          break;
      }
    }
    return iter;
  }

  void DynamicSystemSolver::checkConstraintsForTermination() {
    term = true;

    for (vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (**i).checkConstraintsForTermination();
      if (term == false)
        return;
    }
  }

  void DynamicSystemSolver::checkImpactsForTermination(double dt) {
    term = true;

    for (vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i) {
      (**i).checkImpactsForTermination(dt);
      if (term == false)
        return;
    }
  }

  void DynamicSystemSolver::updateh(double t, int j) {
    h[j].init(0);
    Group::updateh(t, j);
  }

  void DynamicSystemSolver::updateh0Fromh1(double t) {
    h[0].init(0);
    Group::updateh0Fromh1(t);
  }

  void DynamicSystemSolver::updateW0FromW1(double t) {
    W[0].init(0);
    Group::updateW0FromW1(t);
  }

  void DynamicSystemSolver::updateV0FromV1(double t) {
    V[0].init(0);
    Group::updateV0FromV1(t);
  }

  Mat DynamicSystemSolver::dhdq(double t, int lb, int ub) {
    if (lb != 0 || ub != 0) {
      assert(lb >= 0);
      assert(ub <= qSize);
    }
    else if (lb == 0 && ub == 0) {
      lb = 0;
      ub = qSize;
    }
    double delta = epsroot();
    Mat J(hSize[0], qSize, INIT, 0.0);
    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t);
    updateh(t);
    Vec hOld = h[0].copy();
    for (int i = lb; i < ub; i++) {
      double qtmp = q(i);
      q(i) += delta;
      updateStateDependentVariables(t);
      updateg(t);
      updategd(t);
      updateT(t);
      updateJacobians(t);
      updateh(t);
      J.col(i) = (h[0] - hOld) / delta;
      q(i) = qtmp;
    }
    h[0] = hOld;

    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t);
    updateh(t);

    return J;
  }

  Mat DynamicSystemSolver::dhdu(double t, int lb, int ub) {
    if (lb != 0 || ub != 0) {
      assert(lb >= 0);
      assert(ub <= uSize[0]);
    }
    else if (lb == 0 && ub == 0) {
      lb = 0;
      ub = uSize[0];
    }
    double delta = epsroot();
    Mat J(hSize[0], uSize[0], INIT, 0.0);
    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t);
    updateh(t);
    Vec hOld = h[0].copy();
    for (int i = lb; i < ub; i++) {
      //cout << "bin bei i=" << i << endl;
      double utmp = u(i);
      u(i) += delta;
      updateStateDependentVariables(t);
      //updateg(t);
      updategd(t);
      //updateT(t); 
      //updateJacobians(t);
      updateh(t);
      J.col(i) = (h[0] - hOld) / delta;
      u(i) = utmp;
    }
    h[0] = hOld;
    updateStateDependentVariables(t);
    updategd(t);
    updateh(t);

    return J;
  }

  Mat DynamicSystemSolver::dhdx(double t) {
    throw MBSimError("Internal error");
  }

  Vec DynamicSystemSolver::dhdt(double t) {
    throw MBSimError("Internal error");
  }

  void DynamicSystemSolver::updateM(double t, int i) {
    M[i].init(0);
    Group::updateM(t, i);
  }

  void DynamicSystemSolver::updateStateDependentVariables(double t) {
    Group::updateStateDependentVariables(t);

    if (integratorExitRequest) { // if the integrator has not exit after a integratorExitRequest
      cout << "MBSim: Integrator has not stopped integration! Terminate NOW the hard way!" << endl;
      H5::FileSerie::deletePIDFiles();
      _exit(1);
    }

    if (exitRequest) { // on exitRequest flush plot files and ask the integrator to exit
      cout << "MBSim: Flushing HDF5 files and ask integrator to terminate!" << endl;
      H5::FileSerie::flushAllFiles();
      integratorExitRequest = true;
    }

    if (H5::FileSerie::getFlushOnes())
      H5::FileSerie::flushAllFiles(); // flush files ones if requested
  }

  void DynamicSystemSolver::updater(double t, int j) {
    r[j] = V[j] * la; // cannot be called locally (hierarchically), because this adds some values twice to r for tree structures
  }

  void DynamicSystemSolver::updatewb(double t, int j) {
    wb.init(0);
    Group::updatewb(t, j);
  }

  void DynamicSystemSolver::updateW(double t, int j) {
    W[j].init(0);
    Group::updateW(t, j);
  }

  void DynamicSystemSolver::updateV(double t, int j) {
    V[j] = W[j];
    Group::updateV(t, j);
  }

  void DynamicSystemSolver::closePlot() {
    if (getPlotFeature(plotRecursive) == enabled) {
      Group::closePlot();
    }
  }

  int DynamicSystemSolver::solveConstraints() {
    if (la.size() == 0)
      return 0;

    if (useOldla)
      initla();
    else
      la.init(0);

    int iter;
    Vec laOld;
    laOld << la;
    iter = (this->*solveConstraints_)(); // solver election
    if (iter >= maxIter) {
      if (INFO) {
        cerr << endl;
        cerr << "Iterations: " << iter << endl;
        cerr << "\nError: no convergence." << endl;
      }
      if (stopIfNoConvergence) {
        if (dropContactInfo)
          dropContactMatrices();
        throw MBSimError("Maximal Number of Iterations reached");
      }
      if (INFO)
        cerr << "Anyway, continuing integration..." << endl;
    }

    if (warnLevel >= 1 && iter > highIter)
      cerr << endl << "Warning: high number of iterations: " << iter << endl;

    if (useOldla)
      savela();

    return iter;
  }

  int DynamicSystemSolver::solveImpacts(double dt) {
    if (la.size() == 0)
      return 0;
    double H = 1;
    if (dt > 0)
      H = dt;

    if (useOldla)
      initla(H);
    else
      la.init(0);
    
    int iter;
    Vec laOld;
    laOld << la;
    iter = (this->*solveImpacts_)(dt); // solver election
    if (iter >= maxIter) {
      if (INFO) {
        cerr << endl;
        cerr << "Iterations: " << iter << endl;
        cerr << "\nError: no convergence." << endl;
      }
      if (stopIfNoConvergence) {
        if (dropContactInfo)
          dropContactMatrices();
        throw MBSimError("Maximal Number of Iterations reached");
      }
      if (INFO)
        cerr << "Anyway, continuing integration..." << endl;
    }

    if (warnLevel >= 1 && iter > highIter)
      cerr << endl << "Warning: high number of iterations: " << iter << endl;

    if (useOldla)
      savela(H);

    return iter;
  }

  void DynamicSystemSolver::computeInitialCondition() {
    updateStateDependentVariables(0);
    updateg(0);
    checkActive(1);
    updategd(0);
    checkActive(2);
    //setUpActiveLinks();
    updateJacobians(0);
    calclaSize(3);
    calcrFactorSize(3);
    updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)), 0);
    updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)), 0);
    updateWRef(WParent[1](Index(0, getuSize(1) - 1), Index(0, getlaSize() - 1)), 1);
    updateVRef(VParent[1](Index(0, getuSize(1) - 1), Index(0, getlaSize() - 1)), 1);
    updatelaRef(laParent(0, laSize - 1));
    updatewbRef(wbParent(0, laSize - 1));
    updaterFactorRef(rFactorParent(0, rFactorSize - 1));
  }

  Vec DynamicSystemSolver::deltau(const Vec &zParent, double t, double dt) {
    if (q() != zParent())
      updatezRef(zParent);

    updater(t); // TODO update should be outside
    updatedu(t, dt);
    return ud[0];
  }

  Vec DynamicSystemSolver::deltaq(const Vec &zParent, double t, double dt) {
    if (q() != zParent())
      updatezRef(zParent);
    updatedq(t, dt);

    return qd;
  }

  Vec DynamicSystemSolver::deltax(const Vec &zParent, double t, double dt) {
    if (q() != zParent()) {
      updatezRef(zParent);
    }
    updatedx(t, dt);
    return xd;
  }

  void DynamicSystemSolver::initz(Vec& z) {
    updatezRef(z);
//    if (READZ0) {
//      q = q0;
//      u = u0;
//      x = x0;
//    }
//    else
      Group::initz();
  }

  int DynamicSystemSolver::solveConstraintsLinearEquations() {
    la = slvLS(G, -(W[0].T() * slvLLFac(LLM[0], h[0]) + wb));
    return 1;
  }

  int DynamicSystemSolver::solveImpactsLinearEquations(double dt) {
    la = slvLS(G, -(gd + W[0].T() * slvLLFac(LLM[0], h[0]) * dt));
    return 1;
  }

  void DynamicSystemSolver::updateG(double t, int j) {
    G << SqrMat(W[j].T() * slvLLFac(LLM[j], V[j]));

    if (checkGSize)
      ; // Gs.resize();
    else if (Gs.cols() != G.size()) {
      static double facSizeGs = 1;
      if (G.size() > limitGSize && fabs(facSizeGs - 1) < epsroot())
        facSizeGs = double(countElements(G)) / double(G.size() * G.size()) * 1.5;
      Gs.resize(G.size(), int(G.size() * G.size() * facSizeGs));
    }
    Gs << G;
  }

  void DynamicSystemSolver::decreaserFactors() {

    for (vector<Link*>::iterator i = linkSetValuedActive.begin(); i != linkSetValuedActive.end(); ++i)
      (*i)->decreaserFactors();
  }

  void DynamicSystemSolver::update(const Vec &zParent, double t, int options) {
    //cout << "update at t = " << t << endl;
    if (q() != zParent())
      updatezRef(zParent);

    updateStateDependentVariables(t);
    updateg(t);
    checkActive(1);
    //setUpActiveLinks();
    if (gActiveChanged() || options == 1) {
      calcgdSize(2); // IB
      calclaSize(2); // IB
      calcrFactorSize(2); // IB

      updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updatelaRef(laParent(0, laSize - 1));
      updategdRef(gdParent(0, gdSize - 1));
      if (impactSolver == RootFinding)
        updateresRef(resParent(0, laSize - 1));
      updaterFactorRef(rFactorParent(0, rFactorSize - 1));

    }
    updategd(t);

    updateT(t);
    updateJacobians(t);
    updateh(t);
    updateM(t);
    facLLM();
    updateW(t);
    updateV(t);
    updateG(t);
  }

  void DynamicSystemSolver::zdot(const Vec &zParent, Vec &zdParent, double t) {
    if (qd() != zdParent()) {
      updatezdRef(zdParent);
    }
    zdot(zParent, t);
  }

  void DynamicSystemSolver::getLinkStatus(VecInt &LinkStatusExt, double t) {
    if (LinkStatusExt.size() < LinkStatusSize)
      LinkStatusExt.resize(LinkStatusSize);
    if (LinkStatus() != LinkStatusExt())
      updateLinkStatusRef(LinkStatusExt);
    updateLinkStatus(t);
  }

  void DynamicSystemSolver::getLinkStatusReg(VecInt &LinkStatusRegExt, double t) {
    if (LinkStatusRegExt.size() < LinkStatusRegSize)
      LinkStatusRegExt.resize(LinkStatusRegSize);
    if (LinkStatusReg() != LinkStatusRegExt())
      updateLinkStatusRegRef(LinkStatusRegExt);
    updateLinkStatusReg(t);
  }

  void DynamicSystemSolver::projectGeneralizedPositions(double t, int mode) {
    int gID = 0;
    int laID = 0;
    int corrID = 0;
    if (mode == 3) { // impact
      gID = 1; // IG
      laID = 4;
      corrID = 1;
    }
    else if (mode == 2) { // opening, slip->stick
      gID = 2; // IB
      laID = 5;
      corrID = 2;
    }
    else if (mode == 1) { // opening
      gID = 2; // IB
      laID = 5;
      corrID = 2;
    }
    else
      throw MBSimError("Internal error");

    calcgSize(gID);
    calccorrSize(corrID);
    updatecorrRef(corrParent(0, corrSize - 1));
    updategRef(gParent(0, gSize - 1));
    updateg(t);
    updatecorr(corrID);
    Vec nu(getuSize());
    calclaSize(laID);
    updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
    updateW(t);
    //Vec corr;
    //corr = g;
    //corr.init(tolProj);
    SqrMat Gv = SqrMat(W[0].T() * slvLLFac(LLM[0], W[0]));
    // TODO: Wv*T check
    int iter = 0;
    while (nrmInf(g - corr) >= tolProj) {
      if (++iter > 500 && min(g) > 0) {
        cout << "---------------------- breche ab ------------------" << endl;
        break;
      }
      Vec mu = slvLS(Gv, -g + W[0].T() * nu + corr);
      Vec dnu = slvLLFac(LLM[0], W[0] * mu) - nu;
      nu += dnu;
      q += T * dnu;
      //Vec mu = slvLS(Gv,corr-g);
      //Vec dq = slvLLFac(LLM[0],W[0]*mu);
      //q += dq;
      updateStateDependentVariables(t);
      updateg(t);
    }
    calclaSize(3);
    updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
    calcgSize(0);
    updategRef(gParent(0, gSize - 1));
  }

  void DynamicSystemSolver::projectGeneralizedVelocities(double t, int mode) {
    int gdID = 0; // IH
    int corrID = 0;
    if (mode == 3) { // impact
      gdID = 3; // IH
      corrID = 4; // IH
    }
    else if (mode == 2) { // opening, slip->stick
      gdID = 3;
      corrID = 4; // IH
    }
    else if (mode == 1) { // opening and stick->slip
      gdID = 3;
      corrID = 4; // IH
    }
    else
      throw MBSimError("Internal error");
    calccorrSize(corrID); // IH
    if (corrSize) {
      calcgdSize(gdID); // IH
      updatecorrRef(corrParent(0, corrSize - 1));
      updategdRef(gdParent(0, gdSize - 1));
      updategd(t);
      updatecorr(corrID);

      calclaSize(gdID);
      updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updateW(t);

      if (laSize) {
        SqrMat Gv = SqrMat(W[0].T() * slvLLFac(LLM[0], W[0]));
        Vec mu = slvLS(Gv, -gd + corr);
        u += slvLLFac(LLM[0], W[0] * mu);
      }
      calclaSize(3);
      updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      calcgdSize(1);
      updategdRef(gdParent(0, gdSize - 1));
    }
  }

  void DynamicSystemSolver::savela(double dt) {
    for (vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      (**i).savela(dt);
  }

  void DynamicSystemSolver::initla(double dt) {
    for (vector<Link*>::iterator i = linkSetValued.begin(); i != linkSetValued.end(); ++i)
      (**i).initla(dt);
  }

  double DynamicSystemSolver::computePotentialEnergy() {
    double Vpot = 0.0;

    vector<Object*>::iterator i;
    for (i = object.begin(); i != object.end(); ++i)
      Vpot += (**i).computePotentialEnergy();

    vector<Link*>::iterator ic;
    for (ic = link.begin(); ic != link.end(); ++ic)
      Vpot += (**ic).computePotentialEnergy();
    return Vpot;
  }

  void DynamicSystemSolver::addElement(Element *element_) {
    Object* object_ = dynamic_cast<Object*>(element_);
    Link* link_ = dynamic_cast<Link*>(element_);
    if (object_)
      addObject(object_);
    else if (link_)
      addLink(link_);
    else {
      throw MBSimError("ERROR (DynamicSystemSolver: addElement()): No such type of Element to add!");
    }
  }

  Element* DynamicSystemSolver::getElement(const string &name) {
    //   unsigned int i1;
    //   for(i1=0; i1<object.size(); i1++) {
    //     if(object[i1]->getName() == name) return (Element*)object[i1];
    //   }
    //   for(i1=0; i1<object.size(); i1++) {
    //     if(object[i1]->getPath() == name) return (Element*)object[i1];
    //   }
    //   unsigned int i2;
    //   for(i2=0; i2<link.size(); i2++) {
    //     if(link[i2]->getName() == name) return (Element*)link[i2];
    //   }
    //   for(i2=0; i2<link.size(); i2++) {
    //     if(link[i2]->getPath() == name) return (Element*)link[i2];
    //   }
    //   unsigned int i3;
    //   for(i3=0; i3<extraDynamic.size(); i3++) {
    //     if(extraDynamic[i3]->getName() == name) return (Element*)extraDynamic[i3];
    //   }
    //   for(i3=0; i3<extraDynamic.size(); i3++) {
    //     if(extraDynamic[i3]->getPath() == name) return (Element*)extraDynamic[i3];
    //   }
    //   if(!(i1<object.size())||!(i2<link.size())||!(i3<extraDynamic.size())) cout << "Error: The DynamicSystemSolver " << this->name <<" comprises no element " << name << "!" << endl; 
    //   assert(i1<object.size()||i2<link.size()||!(i3<extraDynamic.size()));
    return NULL;
  }

  string DynamicSystemSolver::getSolverInfo() {
    stringstream info;

    if (impactSolver == GaussSeidel)
      info << "GaussSeidel";
    else if (impactSolver == LinearEquations)
      info << "LinearEquations";
    else if (impactSolver == FixedPointSingle)
      info << "FixedPointSingle";
    else if (impactSolver == FixedPointTotal)
      info << "FixedPointTotal";
    else if (impactSolver == RootFinding)
      info << "RootFinding";

    // Gauss-Seidel & solveLL do not depend on the following ...
    if (impactSolver != GaussSeidel && impactSolver != LinearEquations) {
      info << "(";

      // r-Factor strategy
      if (strategy == global)
        info << "global";
      else if (strategy == local)
        info << "local";

      // linear algebra for RootFinding only
      if (impactSolver == RootFinding) {
        info << ",";
        if (linAlg == LUDecomposition)
          info << "LU";
        else if (linAlg == LevenbergMarquardt)
          info << "LM";
        else if (linAlg == PseudoInverse)
          info << "PI";
      }
      info << ")";
    }
    return info.str();
  }

  void DynamicSystemSolver::dropContactMatrices() {
    cout << "dropping contact matrices to file <dump_matrices.asc>" << endl;
    ofstream contactDrop("dump_matrices.asc");

    contactDrop << "constraint functions g" << endl << g << endl << endl;
    contactDrop << endl;
    contactDrop << "mass matrix M" << endl << M[0] << endl << endl;
    contactDrop << "generalized force vector h" << endl << h[0] << endl << endl;
    contactDrop << "generalized force directions W" << endl << W[0] << endl << endl;
    contactDrop << "generalized force directions V" << endl << V[0] << endl << endl;
    contactDrop << "Delassus matrix G" << endl << G << endl << endl;
    contactDrop << "vector wb" << endl << wb << endl << endl;
    contactDrop << endl;
    contactDrop << "constraint velocities gp" << endl << gd << endl << endl;
    contactDrop << "non-holonomic part in gp; b" << endl << b << endl << endl;
    contactDrop << "Lagrange multipliers la" << endl << la << endl << endl;
    contactDrop.close();
  }

  void DynamicSystemSolver::sigInterruptHandler(int) {
    cout << "MBSim: Received user interrupt or terminate signal!" << endl;
    exitRequest = true;
  }

  void DynamicSystemSolver::sigAbortHandler(int) {
    cout << "MBSim: Received abort signal! Flushing HDF5 files and abort!" << endl;
    H5::FileSerie::flushAllFiles();
  }

  void DynamicSystemSolver::writez(string fileName, bool formatH5) {
    if (formatH5) {
      H5::H5File file(fileName, H5F_ACC_TRUNC);

      Group::writez(file);

      file.close();
    }
    else {
      ofstream file(fileName.c_str());
      file.setf(ios::scientific);
      file.precision(numeric_limits<double>::digits10 + 1);
      for (int i = 0; i < q.size(); i++)
        file << q(i) << endl;
      for (int i = 0; i < u.size(); i++)
        file << u(i) << endl;
      for (int i = 0; i < x.size(); i++)
        file << x(i) << endl;
      file.close();
    }
  }

  void DynamicSystemSolver::readz0(string fileName) {
    H5::H5File file(fileName, H5F_ACC_RDONLY);

    DynamicSystem::readz0(file);

    file.close();

    READZ0 = true;
  }

  void DynamicSystemSolver::updatezRef(const Vec &zParent) {

    q >> (zParent(0, qSize - 1));
    u >> (zParent(qSize, qSize + uSize[0] - 1));
    x >> (zParent(qSize + uSize[0], qSize + uSize[0] + xSize - 1));

    updateqRef(q);
    updateuRef(u);
    updatexRef(x);
  }

  void DynamicSystemSolver::updatezdRef(const Vec &zdParent) {

    qd >> (zdParent(0, qSize - 1));
    ud[0] >> (zdParent(qSize, qSize + uSize[0] - 1));
    xd >> (zdParent(qSize + uSize[0], qSize + uSize[0] + xSize - 1));

    updateqdRef(qd);
    updateudRef(ud[0]);
    updatexdRef(xd);

    updateudallRef(ud[0]);
  }

  void DynamicSystemSolver::updaterFactors() {
    if (strategy == global) {
      //     double rFac;
      //     if(G.size() == 1) rFac = 1./G(0,0);
      //     else {
      //       Vec eta = eigvalSel(G,1,G.size());
      //       double etaMax = eta(G.size()-1);
      //       double etaMin = eta(0);
      //       int i=1;
      //       while(abs(etaMin) < 1e-8 && i<G.size()) etaMin = eta(i++);
      //       rFac = 2./(etaMax + etaMin);
      //     }
      //     rFactor.init(rFac);

      throw MBSimError("ERROR (DynamicSystemSolver::updaterFactors()): Global r-Factor strategy not currently not available.");
    }
    else if (strategy == local)
      Group::updaterFactors();
    else
      throw MBSimError("ERROR (DynamicSystemSolver::updaterFactors()): Unknown strategy.");
  }

  void DynamicSystemSolver::computeConstraintForces(double t) {
    la = slvLS(G, -(W[0].T() * slvLLFac(LLM[0], h[0]) + wb)); // slvLS because of undeterminded system of equations
  }

  void DynamicSystemSolver::constructor() {
    integratorExitRequest = false;
    setPlotFeatureRecursive(plotRecursive, enabled);
    setPlotFeature(separateFilePerGroup, enabled);
    setPlotFeatureForChildren(separateFilePerGroup, disabled);
    setPlotFeatureRecursive(openMBV, enabled);
  }

  void DynamicSystemSolver::initializeUsingXML(TiXmlElement *element) {
    // If enviornment variable MBSIMREORGANIZEHIERARCHY=false then do NOT reorganize.
    // In this case it is not possible to simulate a relativ kinematics (tree structures).
    char *reorg=getenv("MBSIMREORGANIZEHIERARCHY");
    if(reorg && strcmp(reorg, "false")==0)
      setReorganizeHierarchy(false);
    else
      setReorganizeHierarchy(true);

    Group::initializeUsingXML(element);
    TiXmlElement *e;
    // search first Environment element
    e = element->FirstChildElement(MBSIMNS"environments")->FirstChildElement();

    while (e) {
      ObjectFactory<Environment>::createAndInit<Environment>(e);
      e = e->NextSiblingElement();
    }

    e = element->FirstChildElement(MBSIMNS"solverParameters");
    if (e) {
      TiXmlElement * ee;
      ee = e->FirstChildElement(MBSIMNS"constraintSolver");
      if (ee) {
        if (ee->FirstChildElement(MBSIMNS"FixedPointTotal"))
          setConstraintSolver(FixedPointTotal);
        else if (ee->FirstChildElement(MBSIMNS"FixedPointSingle"))
          setConstraintSolver(FixedPointSingle);
        else if (ee->FirstChildElement(MBSIMNS"GaussSeidel"))
          setConstraintSolver(GaussSeidel);
        else if (ee->FirstChildElement(MBSIMNS"LinearEquations"))
          setConstraintSolver(LinearEquations);
        else if (ee->FirstChildElement(MBSIMNS"RootFinding"))
          setConstraintSolver(RootFinding);
      }
      ee = e->FirstChildElement(MBSIMNS"impactSolver");
      if (ee) {
        if (ee->FirstChildElement(MBSIMNS"FixedPointTotal"))
          setImpactSolver(FixedPointTotal);
        else if (ee->FirstChildElement(MBSIMNS"FixedPointSingle"))
          setImpactSolver(FixedPointSingle);
        else if (ee->FirstChildElement(MBSIMNS"GaussSeidel"))
          setImpactSolver(GaussSeidel);
        else if (ee->FirstChildElement(MBSIMNS"LinearEquations"))
          setImpactSolver(LinearEquations);
        else if (ee->FirstChildElement(MBSIMNS"RootFinding"))
          setImpactSolver(RootFinding);
      }
      ee = e->FirstChildElement(MBSIMNS"numberOfMaximalIterations");
      if (ee)
        setMaxIter(atoi(ee->GetText()));
      ee = e->FirstChildElement(MBSIMNS"tolerances");
      if (ee) {
        TiXmlElement * eee;
        eee = ee->FirstChildElement(MBSIMNS"projection");
        if (eee)
          setProjectionTolerance(getDouble(eee));
        eee = ee->FirstChildElement(MBSIMNS"g");
        if (eee)
          setgTol(getDouble(eee));
        eee = ee->FirstChildElement(MBSIMNS"gd");
        if (eee)
          setgdTol(getDouble(eee));
        eee = ee->FirstChildElement(MBSIMNS"gdd");
        if (eee)
          setgddTol(getDouble(eee));
        eee = ee->FirstChildElement(MBSIMNS"la");
        if (eee)
          setlaTol(getDouble(eee));
        eee = ee->FirstChildElement(MBSIMNS"La");
        if (eee)
          setLaTol(getDouble(eee));
      }
    }
    e = element->FirstChildElement(MBSIMNS"inverseKinetics");
    if (e)
      setInverseKinetics(Element::getBool(e));
  }

  TiXmlElement* DynamicSystemSolver::writeXMLFile(TiXmlNode *parent) {
    TiXmlElement *ele0 = Group::writeXMLFile(parent);

    TiXmlElement *ele1 = new TiXmlElement(MBSIMNS"environments");
    MBSimEnvironment::getInstance()->writeXMLFile(ele1);
    ele0->LinkEndChild(ele1);

    ele1 = new TiXmlElement(MBSIMNS"solverParameters");
    if (contactSolver != FixedPointSingle) {
      TiXmlElement *ele2 = new TiXmlElement(MBSIMNS"constraintSolver");
      if (contactSolver == FixedPointTotal)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"FixedPointTotal"));
      else if (contactSolver == FixedPointSingle)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"FixedPointSingle"));
      else if (contactSolver == GaussSeidel)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"GaussSeidel"));
      else if (contactSolver == LinearEquations)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"LinearEquations"));
      else if (contactSolver == RootFinding)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"RootFinding"));
      ele1->LinkEndChild(ele2);
    }
    if (impactSolver != FixedPointSingle) {
      TiXmlElement *ele2 = new TiXmlElement(MBSIMNS"impactSolver");
      if (impactSolver == FixedPointTotal)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"FixedPointTotal"));
      else if (impactSolver == FixedPointSingle)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"FixedPointSingle"));
      else if (impactSolver == GaussSeidel)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"GaussSeidel"));
      else if (impactSolver == LinearEquations)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"LinearEquations"));
      else if (impactSolver == RootFinding)
        ele2->LinkEndChild(new TiXmlElement(MBSIMNS"RootFinding"));
      ele1->LinkEndChild(ele2);
    }
    if (maxIter != 10000)
      addElementText(ele1, MBSIMNS"numberOfMaximalIterations", maxIter);
    TiXmlElement *ele2 = new TiXmlElement(MBSIMNS"tolerances");
    if (tolProj > 1e-15)
      addElementText(ele2, MBSIMNS"projection", tolProj);
    if (gTol > 1e-8)
      addElementText(ele2, MBSIMNS"g", gTol);
    if (gdTol > 1e-10)
      addElementText(ele2, MBSIMNS"gd", gdTol);
    if (gddTol > 1e-12)
      addElementText(ele2, MBSIMNS"gdd", gddTol);
    if (laTol > 1e-12)
      addElementText(ele2, MBSIMNS"la", laTol);
    if (LaTol > 1e-10)
      addElementText(ele2, MBSIMNS"La", LaTol);
    ele1->LinkEndChild(ele2);
    ele0->LinkEndChild(ele1);
    if (inverseKinetics)
      addElementText(ele0, MBSIMNS"inverseKinetics", inverseKinetics);

    return ele0;
  }

  DynamicSystemSolver* DynamicSystemSolver::readXMLFile(const string &filename) {
    TiXmlDocument doc;
    bool ret = doc.LoadFile(filename);
    assert(ret == true);
    (void) ret;
    TiXml_PostLoadFile(&doc);
    TiXmlElement *e = doc.FirstChildElement();
    TiXml_setLineNrFromProcessingInstruction(e);
    map<string, string> dummy;
    incorporateNamespace(doc.FirstChildElement(), dummy);
    DynamicSystemSolver *dss = dynamic_cast<DynamicSystemSolver*>(ObjectFactory<Element>::createAndInit<Group>(e));
    return dss;
  }

  void DynamicSystemSolver::writeXMLFile(const string &name) {
    TiXmlDocument doc;
    TiXmlDeclaration *decl = new TiXmlDeclaration("1.0", "UTF-8", "");
    doc.LinkEndChild(decl);
    writeXMLFile(&doc);
    map<string, string> nsprefix = XMLNamespaceMapping::getNamespacePrefixMapping();
    unIncorporateNamespace(doc.FirstChildElement(), nsprefix);
    doc.SaveFile((name.length() > 10 && name.substr(name.length() - 10, 10) == ".mbsim.xml") ? name : name + ".mbsim.xml");
  }

  void DynamicSystemSolver::addToGraph(Graph* graph, SqrMat &A, int i, vector<Object*>& objList) {
    graph->addObject(objList[i]->computeLevel(), objList[i]);
    A(i, i) = -1;

    for (int j = 0; j < A.cols(); j++)
      if (A(i, j) > 0 && fabs(A(j, j) + 1) > epsroot()) // child node of object i
        addToGraph(graph, A, j, objList);
  }

  void DynamicSystemSolver::shift(Vec &zParent, const VecInt &jsv_, double t) {
    if (q() != zParent()) {
      updatezRef(zParent);
    }
    jsv = jsv_;

    checkRoot();
    int maxj = getRootID();
    if (maxj == 3) { // impact (velocity jump)
      checkActive(6); // decide which contacts have closed
      //cout << "stoss" << endl;

      calcgdSize(1); // IG
      updategdRef(gdParent(0, gdSize - 1));
      calclaSize(1); // IG
      calcrFactorSize(1); // IG
      updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updatelaRef(laParent(0, laSize - 1));
      updaterFactorRef(rFactorParent(0, rFactorSize - 1));

      updateStateDependentVariables(t); // TODO necessary?
      updateg(t); // TODO necessary?
      updategd(t); // important because of updategdRef
      updateJacobians(t);
      //updateh(t); // not relevant for impact
      updateM(t);
      updateW(t); // important because of updateWRef
      V[0] = W[0]; //updateV(t) not allowed here
      updateG(t);
      //updatewb(t); // not relevant for impact

      b << gd; // b = gd + trans(W)*slvLLFac(LLM,h)*dt with dt=0
      solveImpacts();
      u += deltau(zParent, t, 0);

      checkActive(3); // neuer Zustand nach Stoss
      // Projektion:
      // - es müssen immer alle Größen projiziert werden
      // - neuer Zustand ab hier bekannt
      // - Auswertung vor Setzen von gActive und gdActive
      updateStateDependentVariables(t); // neues u berücksichtigen
      updateJacobians(t); // für Berechnung von W
      projectGeneralizedPositions(t, 3);
      // Projektion der Geschwindikgeiten erst am Schluss
      //updateStateDependentVariables(t); // Änderung der Lageprojetion berücksichtigen, TODO, prüfen ob notwendig
      //updateJacobians(t);
      //projectGeneralizedVelocities(t,3);

      if (laSize) {

        calclaSize(3); // IH
        calcrFactorSize(3); // IH
        updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
        updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
        updatelaRef(laParent(0, laSize - 1));
        updatewbRef(wbParent(0, laSize - 1));
        updaterFactorRef(rFactorParent(0, rFactorSize - 1));

        //updateStateDependentVariables(t); // necessary because of velocity change 
        updategd(t); // necessary because of velocity change 
        updateJacobians(t);
        updateh(t);
        updateW(t);
        updateV(t);
        updateG(t);
        updatewb(t);
        b << W[0].T() * slvLLFac(LLM[0], h[0]) + wb;
        solveConstraints();

        checkActive(4);
        projectGeneralizedPositions(t, 2);
        updateStateDependentVariables(t); // necessary because of velocity change 
        updateJacobians(t);
        projectGeneralizedVelocities(t, 2);
        
      }
    }
    else if (maxj == 2) { // transition from slip to stick (acceleration jump)
      //cout << "haften" << endl;
      checkActive(7); // decide which contacts may stick

      calclaSize(3); // IH
      calcrFactorSize(3); // IH
      updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
      updatelaRef(laParent(0, laSize - 1));
      updatewbRef(wbParent(0, laSize - 1));
      updaterFactorRef(rFactorParent(0, rFactorSize - 1));

      if (laSize) {
        updateStateDependentVariables(t); // TODO necessary
        updateg(t); // TODO necessary
        updategd(t); // TODO necessary
        updateT(t);  // TODO necessary
        updateJacobians(t);
        updateh(t);  // TODO necessary
        updateM(t);  // TODO necessary
        facLLM();  // TODO necessary 
        updateW(t);  // TODO necessary
        updateV(t);  // TODO necessary
        updateG(t);  // TODO necessary 
        updatewb(t);  // TODO necessary 
        b << W[0].T() * slvLLFac(LLM[0], h[0]) + wb;
        solveConstraints();

        checkActive(4);

        projectGeneralizedPositions(t, 2);
        updateStateDependentVariables(t); // Änderung der Lageprojetion berücksichtigen, TODO, prüfen ob notwendig
        updateJacobians(t);
        projectGeneralizedVelocities(t, 2);

      }
    }
    else if (maxj == 1) { // contact opens or transition from stick to slip
      checkActive(8);

      projectGeneralizedPositions(t, 1);
      updateStateDependentVariables(t); // Änderung der Lageprojetion berücksichtigen, TODO, prüfen ob notwendig
      updateJacobians(t);
      projectGeneralizedVelocities(t, 1);
    }
    checkActive(5); // final update von gActive, ...
    calclaSize(3); // IH
    calcrFactorSize(3); // IH
    updateWRef(WParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
    updateVRef(VParent[0](Index(0, getuSize() - 1), Index(0, getlaSize() - 1)));
    updatelaRef(laParent(0, laSize - 1));
    updatewbRef(wbParent(0, laSize - 1));
    updaterFactorRef(rFactorParent(0, rFactorSize - 1));

    updateStateDependentVariables(t);
    updateJacobians(t);
    updateg(t);
    updategd(t);
    setRootID(0);
  }

  void DynamicSystemSolver::getsv(const Vec& zParent, Vec& svExt, double t) {
    if (sv() != svExt()) {
      updatesvRef(svExt);
    }

    if (q() != zParent())
      updatezRef(zParent);

    if (qd() != zdParent())
      updatezdRef(zdParent);

    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t);
    updateh(t);
    updateM(t);
    facLLM();
    if (laSize) {
      updateW(t);
      updateV(t);
      updateG(t);
      updatewb(t);
      b << W[0].T() * slvLLFac(LLM[0], h[0]) + wb;
      solveConstraints();
    }
    updateStopVector(t);
    //sv(sv.size()-1) = driftCount*1e-0-t; 
    sv(sv.size() - 1) = 1;
  }

  Vec DynamicSystemSolver::zdot(const Vec &zParent, double t) {
    if (q() != zParent()) {
      updatezRef(zParent);
    }
    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t);
    updateh(t);
    updateM(t);
    facLLM();
    if (laSize) {
      updateW(t);
      updateV(t);
      updateG(t);
      updatewb(t);
      computeConstraintForces(t);
    }
    updater(t);
    updatezd(t);

    return zdParent;
  }

  void DynamicSystemSolver::plot(const fmatvec::Vec& zParent, double t, double dt) {
    if (q() != zParent()) {
      updatezRef(zParent);
    }

    if (qd() != zdParent())
      updatezdRef(zdParent);
    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateT(t);
    updateJacobians(t, 0);
    updateJacobians(t, 1);
    updateh(t, 1);
    updateh0Fromh1(t);
    updateM(t, 0);
    facLLM(0);
    updateWRef(WParent[1](Index(0, getuSize(1) - 1), Index(0, getlaSize() - 1)), 1);
    updateVRef(VParent[1](Index(0, getuSize(1) - 1), Index(0, getlaSize() - 1)), 1);
    if (laSize) {

      updateW(t, 1);
      updateV(t, 1);
      updateWnVRefObjects();
      updateW0FromW1(t);
      updateV0FromV1(t);
      updateG(t);
      updatewb(t);
      computeConstraintForces(t);
    }

    updater(t, 0);
    updater(t, 1);
    updatezd(t);
    if (true) {
      updateStateDerivativeDependentVariables(t); // TODO: verbinden mit updatehInverseKinetics

      updatehInverseKinetics(t, 1); // Accelerations of objects

      updategInverseKinetics(t); // necessary because of update of force direction
      updategdInverseKinetics(t); // necessary because of update of force direction
      updateJacobiansInverseKinetics(t, 1);
      updateWInverseKinetics(t, 1);
      updatebInverseKinetics(t);

      int n = WInverseKinetics[1].cols();
      int m1 = WInverseKinetics[1].rows();
      int m2 = bInverseKinetics.rows();
      Mat A(m1 + m2, n);
      Vec b(m1 + m2);
      A(Index(0, m1 - 1), Index(0, n - 1)) = WInverseKinetics[1];
      A(Index(m1, m1 + m2 - 1), Index(0, n - 1)) = bInverseKinetics;
      b(0, m1 - 1) = -h[1] - r[1];
      laInverseKinetics = slvLL(JTJ(A), A.T() * b);
    }

    DynamicSystemSolver::plot(t, dt);

    if (++flushCount > flushEvery) {
      flushCount = 0;
      H5::FileSerie::flushAllFiles();
    }
  }

  // TODO: Momentan für TimeStepping benötigt
  void DynamicSystemSolver::plot2(const fmatvec::Vec& zParent, double t, double dt) {
    if (q() != zParent()) {
      updatezRef(zParent);
    }

    if (qd() != zdParent())
      updatezdRef(zdParent);

    updateStateDependentVariables(t);
    updateg(t);
    updategd(t);
    updateJacobians(t);
    updateT(t);
    updateh(t);
    updateM(t);
    facLLM();
    updateW(t);

    plot(t, dt);

    if (++flushCount > flushEvery) {
      flushCount = 0;
      H5::FileSerie::flushAllFiles();
    }
  }

}
