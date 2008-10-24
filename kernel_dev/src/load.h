/* Copyright (C) 2004-2006  Martin Förg
 
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
 * Contact:
 *   mfoerg@users.berlios.de
 *
 */

#ifndef _LOAD_H_
#define _LOAD_H_

#include "link.h"

namespace MBSim {

  class DataInterfaceBase;

  /*! Comment
   *
   * */
  class Load : public Link {

    protected:
      Index IT, IR;
      Mat forceDir, momentDir, Wf, Wm;

      DataInterfaceBase *func;
      int KOSYID;

      void updateg(double t) {}
      void updategd(double t) {}
      void updateh(double t);

    public: 
      Load(const string &name);
      virtual ~Load();

      void calclaSize();
      void init();
      bool isActive() const {return true;}
      void checkHolonomicConstraints() {}
      void checkNonHolonomicConstraints() {}
      bool activeConstraintsChanged() {return false;}
      bool activeHolonomicConstraintsChanged() {return false;}
      bool activeNonHolonomicConstraintsChanged() {return false;}


      void setKOSY(int);
      void setUserFunction(DataInterfaceBase *func_);
      void setSignal(DataInterfaceBase *func_);
      void connect(CoordinateSystem *port1);
      void setForceDirection(const Mat& fd);
      void setMomentDirection(const Mat& md);

      void initDataInterfaceBase(MultiBodySystem *parentmbs);
  };

}

#endif
