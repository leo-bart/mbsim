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

#ifndef _PARAMETER__H_
#define _PARAMETER__H_

#include "treeitemdata.h"
#include "parameter_property_dialog.h"
#include "parameter_context_menu.h"

namespace XERCES_CPP_NAMESPACE {
  class DOMElement;
  class DOMNode;
}

namespace MBSimGUI {

  class EmbedItemData;
  class PropertyWidget;
  class PropertyDialog;
  class ExtWidget;
  class TextWidget;

  class Parameter : public TreeItemData {
    public:
      Parameter(const QString &name="") : TreeItemData(name), parent(NULL), config(false) { }
      xercesc::DOMElement* createXMLElement(xercesc::DOMNode *parent);
      static void insertXMLElement(xercesc::DOMElement *element, xercesc::DOMNode *parent);
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual ParameterPropertyDialog* createPropertyDialog() { return new ParameterPropertyDialog(this); }
      virtual ParameterContextMenu* createContextMenu() { return new ParameterContextMenu(this); }
      xercesc::DOMElement* getXMLElement() { return element; }
      virtual void removeXMLElements();
      EmbedItemData* getParent() { return parent; }
      void setParent(EmbedItemData* parent_) { parent = parent_; }
      bool getConfig() { return config; }
      void setConfig(bool config_) { config = config_; }
      static std::vector<Parameter*> initializeParametersUsingXML(xercesc::DOMElement *element);
    protected:
      EmbedItemData *parent;
      QString name, valuestr;
      xercesc::DOMElement *element;
      bool config;
  };

  class StringParameter : public Parameter {
    public:
      StringParameter(const QString &name="") : Parameter(name) { }
      virtual QString getType() const { return "stringParameter"; }
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual ParameterPropertyDialog* createPropertyDialog() {return new StringParameterPropertyDialog(this);}
  };

  class ScalarParameter : public Parameter {
    public:
      ScalarParameter(const QString &name="") : Parameter(name) { }
      virtual QString getType() const { return "scalarParameter"; }
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual ParameterPropertyDialog* createPropertyDialog() {return new ScalarParameterPropertyDialog(this);}
  };

  class VectorParameter : public Parameter {
    public:
      VectorParameter(const QString &name="") : Parameter(name) { }
      virtual QString getType() const { return "vectorParameter"; }
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual ParameterPropertyDialog* createPropertyDialog() {return new VectorParameterPropertyDialog(this);}
  };

  class MatrixParameter : public Parameter {
    public:
      MatrixParameter(const QString &name="") : Parameter(name) { }
      virtual QString getType() const { return "matrixParameter"; }
      virtual void initializeUsingXML(xercesc::DOMElement *element);
      virtual ParameterPropertyDialog* createPropertyDialog() {return new MatrixParameterPropertyDialog(this);}
  };

  class ImportParameter : public Parameter {
    public:
      ImportParameter() : Parameter("") { }
      virtual QString getType() const { return "import"; }
      virtual ParameterPropertyDialog* createPropertyDialog() {return new ImportParameterPropertyDialog(this);}
  };

}

#endif
