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

//#include "object.h"
//#include "link.h"
//#include "solver.h"
//#include "integrator.h"
//#include "parameter.h"
//#include "observer.h"
#include <set>
#include <iostream>
#include <map>
class Element;
class Frame;
class Contour;
class Object;
class Link;
class Group;
class Observer;
class Integrator;
class Parameter;
class Environment;
class TiXmlElement;
class TreeItem;

class ObjectFactoryBase {
  protected:
    ObjectFactoryBase() {}
    virtual ~ObjectFactoryBase() {}
    typedef std::pair<std::string, std::string> P_NSPRE;
    typedef std::map<std::string, std::string> M_NSPRE;
    typedef std::pair<double, P_NSPRE> P_PRINSPRE;
  public:
    typedef std::multimap<double, P_NSPRE> MM_PRINSPRE;
    virtual Contour* createContour(TiXmlElement *element, Element *parent) { return NULL; }
    virtual Group* createGroup(TiXmlElement *element, Element *parent) { return NULL; }
    virtual Object* createObject(TiXmlElement *element, Element *parent) { return NULL; }
  //  virtual ExtraDynamic * createExtraDynamic(TiXmlElement *element) { return NULL; }
  //  virtual Translation* createTranslation(TiXmlElement *element) { return NULL; }
  //  virtual Rotation* createRotation(TiXmlElement *element) { return NULL; }
  virtual Link* createLink(TiXmlElement *element, Element *parent) { return NULL; }
  virtual Observer* createObserver(TiXmlElement *element, Element *parent) { return NULL; }
  virtual Integrator* createIntegrator(TiXmlElement *element) { return NULL; }
  virtual Parameter* createParameter(TiXmlElement *element) { return NULL; }
  //  virtual GeneralizedForceLaw *createGeneralizedForceLaw(TiXmlElement *element) { return NULL; }
  //  virtual GeneralizedImpactLaw *createGeneralizedImpactLaw(TiXmlElement *element) { return NULL; }
  //  virtual FrictionForceLaw *createFrictionForceLaw(TiXmlElement *element) { return NULL; }
  //  virtual FrictionImpactLaw *createFrictionImpactLaw(TiXmlElement *element) { return NULL; }
  //  virtual Contour *createContour(TiXmlElement *element) { return NULL; }
    virtual Environment *getEnvironment(TiXmlElement *element) { return NULL; }
  //  virtual Jacobian *createJacobian(TiXmlElement *element) { return NULL; }
  //  virtual Function1<double,double> *createFunction1_SS(TiXmlElement *element) { return NULL; }
  //  virtual Function1<fmatvec::Vec,double> *createFunction1_VS(TiXmlElement *element) { return NULL; }
  //  virtual Function1<fmatvec::Vec3,double> *createFunction1_V3S(TiXmlElement *element) { return NULL; }
  //  virtual Function2<double,double,double> *createFunction2_SSS(TiXmlElement *element) { return NULL; }
  //  virtual Function2<fmatvec::Vec,fmatvec::Vec,double> *createFunction2_VVS(TiXmlElement *element) { return NULL; }
  //  virtual Function3<fmatvec::Mat3V,fmatvec::Vec,fmatvec::Vec,double> *createFunction3_MVVS(TiXmlElement *element) { return NULL; }
  //  virtual ContourFunction1s * createContourFunction1s(TiXmlElement * element) { return NULL; }
    virtual MM_PRINSPRE& getPriorityNamespacePrefix() {
      static MM_PRINSPRE ret;
      return ret;
    }
};

class ObjectFactory : public ObjectFactoryBase {
  protected:
    ObjectFactory() {}
    virtual ~ObjectFactory() {}
  private:
    static ObjectFactory *instance;
    std::set<ObjectFactoryBase*> factories;
  public:
    static ObjectFactory* getInstance() { return instance?instance:instance=new ObjectFactory; }
    void registerObjectFactory(ObjectFactoryBase *fac) { factories.insert(fac); }
    void unregisterObjectFactory(ObjectFactory *fac) { factories.erase(fac); }

    Contour* createContour(TiXmlElement *element, Element *parent);
    Group* createGroup(TiXmlElement *element, Element *parent);
    Object* createObject(TiXmlElement *element, Element *parent);
//    ExtraDynamic * createExtraDynamic(TiXmlElement *element);
//    Translation* createTranslation(TiXmlElement *element);
//    Rotation* createRotation(TiXmlElement *element);
    Link* createLink(TiXmlElement *element, Element *parent);
    Observer* createObserver(TiXmlElement *element, Element *parent);
    Integrator* createIntegrator(TiXmlElement *element);
    Parameter* createParameter(TiXmlElement *element);
//    GeneralizedForceLaw *createGeneralizedForceLaw(TiXmlElement *element);
//    GeneralizedImpactLaw *createGeneralizedImpactLaw(TiXmlElement *element);
//    FrictionForceLaw *createFrictionForceLaw(TiXmlElement *element);
//    FrictionImpactLaw *createFrictionImpactLaw(TiXmlElement *element);
    Environment *getEnvironment(TiXmlElement *element);
//    Jacobian *createJacobian(TiXmlElement *element);
//    Function1<double,double> *createFunction1_SS(TiXmlElement *element);
//    Function1<fmatvec::Vec,double> *createFunction1_VS(TiXmlElement *element);
//    Function1<fmatvec::Vec3,double> *createFunction1_V3S(TiXmlElement *element);
//    Function2<double,double,double> *createFunction2_SSS(TiXmlElement *element);
//    Function2<fmatvec::Vec,fmatvec::Vec,double> *createFunction2_VVS(TiXmlElement *element);
//    Function3<fmatvec::Mat3V,fmatvec::Vec,fmatvec::Vec,double> *createFunction3_MVVS(TiXmlElement *element);
//    ContourFunction1s * createContourFunction1s(TiXmlElement * element);
    M_NSPRE getNamespacePrefixMapping();
};


class MBSimObjectFactory : protected ObjectFactoryBase  {
  private:
    static MBSimObjectFactory *instance;
    MBSimObjectFactory() {}
  public:
    // This static function must be called before the ObjectFactory is used to create
    // objects from MBSimObjectFactory
    static void initialize();
  protected:
    Contour* createContour(TiXmlElement *element, Element *parent);
    Group* createGroup(TiXmlElement *element, Element *parent);
    Object* createObject(TiXmlElement *element, Element *parent);
//    ExtraDynamic * createExtraDynamic(TiXmlElement *element) {return 0; }
//    Translation* createTranslation(TiXmlElement *element);
//    Rotation* createRotation(TiXmlElement *element);
    Link* createLink(TiXmlElement *element, Element *parent);
    Observer* createObserver(TiXmlElement *element, Element *parent);
    Integrator* createIntegrator(TiXmlElement *element);
    Parameter* createParameter(TiXmlElement *element);
//    GeneralizedForceLaw *createGeneralizedForceLaw(TiXmlElement *element);
//    GeneralizedImpactLaw *createGeneralizedImpactLaw(TiXmlElement *element);
//    FrictionForceLaw *createFrictionForceLaw(TiXmlElement *element);
//    FrictionImpactLaw *createFrictionImpactLaw(TiXmlElement *element);
    Environment *getEnvironment(TiXmlElement *element);
//    Jacobian *createJacobian(TiXmlElement *element);
//    Function1<double,double> *createFunction1_SS(TiXmlElement *element);
//    Function1<fmatvec::Vec,double> *createFunction1_VS(TiXmlElement *element);
//    Function1<fmatvec::Vec3,double> *createFunction1_V3S(TiXmlElement *element);
//    Function2<double,double,double> *createFunction2_SSS(TiXmlElement *element);
//    Function2<fmatvec::Vec,fmatvec::Vec,double> *createFunction2_VVS(TiXmlElement *element);
//    Function3<fmatvec::Mat3V,fmatvec::Vec,fmatvec::Vec,double> *createFunction3_MVVS(TiXmlElement *element);
//    ContourFunction1s * createContourFunction1s(TiXmlElement * element) {return 0; }
};
