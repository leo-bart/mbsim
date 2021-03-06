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

#ifndef __MAINWINDOW_H_
#define __MAINWINDOW_H_

#include <QMainWindow>
#include <QProcess>
#include <boost/filesystem/path.hpp>
#include <xercesc/util/XercesDefs.hpp>
#include <deque>

class QAction;
class QModelIndex;
class QWebView;

namespace OpenMBVGUI {
  class MainWindow;
}

namespace MBXMLUtils {
  class Eval;
  class NewParamLevel;
  class DOMParser;
}

namespace XERCES_CPP_NAMESPACE {
  class DOMDocument;
  class DOMElement;
}

namespace MBSimGUI {

  class Process;
  class MBSimThread;
  class ElementView;
  class EmbeddingView;
  class SolverView;
  class Element;
  class Frame;
  class Contour;
  class Group;
  class Object;
  class Link;
  class Constraint;
  class Observer;
  class Parameter;
  class EmbedItemData;

  class MainWindow : public QMainWindow {

    Q_OBJECT

    private:
      ElementView *elementList;
      EmbeddingView *embeddingList;
      SolverView *solverView;
      QString fileProject; 
      Process *mbsim;
      MBSimThread *mbsimThread;
      OpenMBVGUI::MainWindow *inlineOpenMBVMW;
      boost::filesystem::path uniqueTempDir;
      QAction *actionSaveProject, *actionSaveMBS, *actionSimulate, *actionOpenMBV, *actionH5plotserie, *actionEigenanalysis, *actionSaveIntegrator, *actionSaveParameterList, *actionSaveDataAs, *actionSaveMBSimH5DataAs, *actionSaveOpenMBVDataAs, *actionRefresh, *actionSaveStateVectorAs, *actionSaveEigenanalysisAs;
      QTimer *autoSaveTimer;
      QString currentID;
      enum { maxRecentFiles = 5 };
      QAction *recentProjectFileActs[maxRecentFiles];
      bool autoSave, autoExport, saveFinalStateVector;
      int autoSaveInterval, maxUndo;
      QString autoExportDir;
      static QDialog *helpDialog;
      static QWebView *helpViewer;
      bool allowUndo;
      xercesc::DOMDocument *doc;
      std::deque<xercesc::DOMDocument*> undos, redos;
      std::pair<Element*,bool> elementBuffer;
      std::pair<Parameter*,bool> parameterBuffer;
      void initInlineOpenMBV();
      void dragEnterEvent(QDragEnterEvent *event);
      void dropEvent(QDropEvent *event);
      void closeEvent(QCloseEvent *event);
      bool maybeSave();
      void setCurrentProjectFile(const QString &fileName);
      void updateRecentProjectFileActions();
      void move(bool up);
      void moveParameter(bool up);
      void moveFrame(bool up);
      void moveContour(bool up);
      void moveGroup(bool up);
      void moveObject(bool up);
      void moveLink(bool up);
      void moveConstraint(bool up);
      void moveObserver(bool up);

    public:
      MainWindow(QStringList &arg);
      ~MainWindow();
      std::shared_ptr<MBXMLUtils::DOMParser> parser;
      std::shared_ptr<MBXMLUtils::Eval> eval;
//      ExtProperty evalSelect;
      void mbsimxml(int task);
      const boost::filesystem::path& getUniqueTempDir() const {return uniqueTempDir;}
      void addParameter(Parameter *parameter, EmbedItemData *parent);
      void addFrame(Frame *frame, Element *parent);
      void addContour(Contour *contour, Element *parent);
      void addGroup(Group *group, Element *parent);
      void addObject(Object *object, Element *parent);
      void addLink(Link *link, Element *parent);
      void addConstraint(Constraint *constraint, Element *parent);
      void addObserver(Observer *observer, Element *parent);
      void loadParameter(EmbedItemData *parent, Parameter *parameter);
      void loadFrame(Element *parent, Element *element=NULL);
      void loadContour(Element *parent, Element *element=NULL);
      void loadGroup(Element *parent, Element *element=NULL);
      void loadObject(Element *parent, Element *element=NULL);
      void loadLink(Element *parent, Element *element=NULL);
      void loadConstraint(Element *parent, Element *element=NULL);
      void loadObserver(Element *parent, Element *element=NULL);
      void highlightObject(const QString &ID);
      const QString& getHighlightedObject() const {return currentID;}
      void loadProject(const QString &file);
      ElementView* getElementList() { return elementList; }
      EmbeddingView* getEmbeddingList() { return embeddingList; }
      void setProjectChanged(bool changed=true);
      void selectSolver(int i);
      void setAllowUndo(bool allowUndo_) { allowUndo = allowUndo_; }
      const std::pair<Element*,bool>& getElementBuffer() const { return elementBuffer; }
      const std::pair<Parameter*,bool>& getParameterBuffer() const { return parameterBuffer; }

    public slots:
      void elementListClicked();
      void parameterListClicked();
      void solverViewClicked();
      void newProject(bool ask=true);
      void loadProject();
      bool saveProjectAs();
      bool saveProject(const QString &filename="", bool processDocument=true, bool modifyStatus=true);
      void saveDataAs();
      void saveMBSimH5DataAs();
      void saveMBSimH5Data(const QString &file);
      void saveOpenMBVDataAs();
      void saveOpenMBVXMLData(const QString &file);
      void saveOpenMBVH5Data(const QString &file);
      void saveStateVectorAs();
      void saveStateVector(const QString &file);
      void saveEigenanalysisAs();
      void saveEigenanalysis(const QString &file);
      void simulate();
      void refresh();
      void openmbv();
      void h5plotserie();
      void eigenanalysis();
      void debug();
      void help();
      void xmlHelp(const QString &url="");
      void about();
      void updateParameters(EmbedItemData *item, bool exceptLatestParameter=false);
      void rebuildTree();
      void undo();
      void redo();
      void removeElement();
      void removeParameter();
      void remove();
      void copy(bool cut=false);
      void cut() { copy(true); }
      void paste();
      void moveUp() { move(true); }
      void moveDown() { move(false); }
      void copyElement(bool cut=false);
      void cutElement() { copyElement(true); }
      void copyParameter(bool cut=false);
      void cutParameter() { copyParameter(true); }
      void moveUpParameter() { moveParameter(true); }
      void moveDownParameter() { moveParameter(false); }
      void moveUpFrame() { moveFrame(true); }
      void moveDownFrame() { moveFrame(false); }
      void moveUpContour() { moveContour(true); }
      void moveDownContour() { moveContour(false); }
      void moveUpGroup() { moveGroup(true); }
      void moveDownGroup() { moveGroup(false); }
      void moveUpObject() { moveObject(true); }
      void moveDownObject() { moveObject(false); }
      void moveUpLink() { moveLink(true); }
      void moveDownLink() { moveLink(false); }
      void moveUpConstraint() { moveConstraint(true); }
      void moveDownConstraint() { moveConstraint(false); }
      void moveUpObserver() { moveObserver(true); }
      void moveDownObserver() { moveObserver(false); }
      void saveElementAs();
      void projectSettings();

    private slots:
      void selectElement(std::string ID);
      void changeWorkingDir();
      void openOptionsMenu();
      void selectionChanged(const QModelIndex &current);
      void simulationFinished(int exitCode, QProcess::ExitStatus exitStatus);
      void openRecentProjectFile();
      void preprocessFinished(int result);
      void autoSaveProject();
      void applySettings();
      void settingsFinished(int result);
  };

}

#endif
