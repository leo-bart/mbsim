/* Copyright (C) 2004-2014 MBSim Development Team
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
 */

#ifndef _SINGLE_CONTACT_H_
#define _SINGLE_CONTACT_H_

#include <mbsim/links/contour_link.h>

namespace MBSim {

  class ContactKinematics;
  class GeneralizedForceLaw;
  class GeneralizedImpactLaw;
  class FrictionForceLaw;
  class FrictionImpactLaw;

  /*! \brief class for contacts
   * \author Martin Foerg
   * \date 2009-04-02 some comments (Thorsten Schindler)
   * \date 2009-07-16 splitted link / object right hand side (Thorsten Schindler)
   * \date 2009-08-03 contacts can now visualize their ContactPointFrames (Markus Schneider)
   * \date 2010-07-06 added LinkStatus and LinearImpactEstimation for timestepper ssc (Robert Huber)
   * \date 2012-05-08 added LinkStatusReg for AutoTimeSteppingSSCIntegrator (Jan Clauberg)
   * \date 2014-09-16 contact forces are calculated on acceleration level (Thorsten Schindler)
   *
   * basic class for contacts between contours, mainly implementing geometrical informations of contact-pairings
   * 
   * Remarks:
   * - constitutive laws on acceleration and velocity level have to be set pairwise
   */
  class SingleContact: public ContourLink {

    friend class Contact;

    public:
      /*!
       * \brief constructor
       * \param name of contact
       */      
      SingleContact(const std::string &name="");

      void resetUpToDate();

      bool isSticking() const;

      const double& evalGeneralizedNormalForce() { if(updlaN) updateGeneralizedNormalForce(); return lambdaN; }
      const fmatvec::VecV& evalGeneralizedTangentialForce() { if(updlaT) updateGeneralizedTangentialForce(); return lambdaT; }

      double& getGeneralizedNormalForce(bool check=true) {  assert((not check) or (not updlaN)); return lambdaN; }

      const double& evallaN();
      const fmatvec::Vec& evallaT();

      const double& evalLaN();
      const fmatvec::Vec& evalLaT();

      const double& evalgdnN();
      const fmatvec::Vec& evalgdnT();

      const double& evalgddN();
      const fmatvec::Vec& evalgddT();

      /* INHERITED INTERFACE OF LINKINTERFACE */
      virtual void updatewb();
      virtual void updateV(int i=0);
      void updateGeneralizedNormalForce() { (this->*updateGeneralizedNormalForce_)(); updlaN = false; }
      void updateGeneralizedTangentialForce() { (this->*updateGeneralizedTangentialForce_)(); updlaT = false; }
      void updateGeneralizedNormalForceS();
      void updateGeneralizedNormalForceM();
      void updateGeneralizedNormalForceP();
      void updateGeneralizedTangentialForceS();
      void updateGeneralizedTangentialForceM();
      void updateGeneralizedTangentialForce0() { }
      void (SingleContact::*updateGeneralizedNormalForce_)();
      void (SingleContact::*updateGeneralizedTangentialForce_)();
      virtual void updateGeneralizedForces();
      virtual void updateGeneralizedPositions();
      virtual void updateGeneralizedVelocities();
      virtual void updatePositions();
      virtual void updatePositions(Frame *frame);
      virtual void updateVelocities();
      virtual void updateg();
      virtual void updategd();
      virtual void updateh(int i=0);
      virtual void updateW(int i=0);
      virtual void updateStopVector();
      /***************************************************/

      /* INHERITED INTERFACE OF LINK */
      virtual void updatelaRef(const fmatvec::Vec& ref);
      virtual void updateLaRef(const fmatvec::Vec& ref);
      virtual void updategdRef(const fmatvec::Vec& ref);
      virtual void calcSize();
      virtual void calcxSize();
      virtual void calclaSize(int j);
      virtual void calcgSize(int j);
      virtual void calcgdSize(int j);
      virtual void calcrFactorSize(int j);
      virtual void calcsvSize();
      virtual void calcLinkStatusSize();
      virtual void calcLinkStatusRegSize();
      virtual void init(InitStage stage);
      virtual bool isSetValued() const;
      virtual bool isSingleValued() const;
      virtual void updateLinkStatus();
      virtual void updateLinkStatusReg();
      virtual bool isActive() const;
      virtual bool gActiveChanged();
      virtual bool detectImpact();
      virtual void solveImpactsFixpointSingle();
      virtual void solveConstraintsFixpointSingle();
      virtual void solveImpactsGaussSeidel();
      virtual void solveConstraintsGaussSeidel();
      virtual void solveImpactsRootFinding();
      virtual void solveConstraintsRootFinding();
      virtual void jacobianConstraints();
      virtual void jacobianImpacts();
      virtual void updaterFactors();
      virtual void checkConstraintsForTermination();
      virtual void checkImpactsForTermination();
      virtual void checkActive(int j);
      virtual void LinearImpactEstimation(double t, fmatvec::Vec &gInActive_,fmatvec::Vec &gdInActive_,int *IndInActive_,fmatvec::Vec &gAct_,int *IndActive_);
      virtual void SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_);
 
      /***************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const { return "Contact"; }
      /***************************************************/

      /* GETTER / SETTER */
      void setNormalForceLaw(GeneralizedForceLaw *fcl_);
      GeneralizedForceLaw * getNormalForceLaw() const {return fcl; }
      void setNormalImpactLaw(GeneralizedImpactLaw *fnil_);
      void setTangentialForceLaw(FrictionForceLaw *fdf_);
      void setTangentialImpactLaw(FrictionImpactLaw *ftil_);
      void setContactKinematics(ContactKinematics* ck) { contactKinematics = ck; }
      ContactKinematics* getContactKinematics() const { return contactKinematics; }
      /***************************************************/

      /**
       * \return number of considered friction directions
       */
      virtual int getFrictionDirections() const;

      /*! connect two contours
       * \param first contour
       * \param second contour
       * \param specify the contact kinematics
       *
       * REMARK: The contact frame of the first contour is used to plot the contacts data in
       */
//      void connect(Contour *contour1, Contour* contour2, ContactKinematics* contactKinematics = 0);

      virtual void initializeUsingXML(xercesc::DOMElement *element);

      void calccorrSize(int j);
      void updatecorr(int j);

      void checkRoot();

    protected:
      /**
       * \brief used contact kinematics
       */
      ContactKinematics *contactKinematics;

      /**
       * \brief force laws in normal and tangential direction on acceleration and velocity level
       */
      GeneralizedForceLaw *fcl;

      /**
       * \brief force law defining relation between tangential velocities and tangential forces
       */
      FrictionForceLaw *fdf;
      
      /**
       * \brief force law defining relation between penetration velocity and resulting normal impulses
      */
      GeneralizedImpactLaw *fnil;
      
      /** 
       * \brief force law defining relation between tangential velocities and forces impulses
      */
      FrictionImpactLaw *ftil;

      /*!
       * \brief force in normal direction
       *
       * \todo: only double needed here
       */
      fmatvec::Vec laN, LaN;

      /*!
       * \brief force in tangential direction
       *
       * \todo: use fixed size?
       */
      fmatvec::Vec laT, LaT;

      /*!
       * \brief relative velocity in normal direction
       *
       */
      fmatvec::Vec gdN;

      /*!
       * \brief relative velocity in tangential direction
       *
       * \todo: use fixed size?
       */
      fmatvec::Vec gdT;

      /** 
       * \brief boolean flag symbolising activity of contact on position level with possibility to save previous time step
       */
      unsigned int gActive, gActive0;

      enum Direction {
        normal,
        tangential,
        DirectionDIM
      };

      /** 
       * \brief boolean flag symbolising activity of contact on velocity level
       *
       * gdActive[normal] = normal direction; gdActive[tangential] = tangential direction
       */
      std::array<unsigned int, DirectionDIM> gdActive;

      /** 
       * \brief boolean flag symbolising activity of contact on acceleration level
       *
       * gddActive[normal] = normal direction; gddActive[tangential] = tangential direction
       */
      std::array<unsigned int, DirectionDIM> gddActive;

      /**
       * \brief new gap velocity after an impact for event driven scheme
       */
      fmatvec::Vec gdnN, gdnT;
      
      /**
       * \brief gap acceleration for event driven scheme on acceleration level
       */
      fmatvec::Vec gddN, gddT;

      /*!
       * \brief gap acceleration buffer
       */
      fmatvec::Vec gddNBuf, gddTBuf;

      double lambdaN;
      fmatvec::VecV lambdaT;

      bool updlaN, updlaT;

      /**
       * \brief type of detected root
       *
       * 1 = close-open transition / stick-slip transition
       * 2 = slip-stick transition
       * 3 = open-close transition, i.e., impact
       */
      int rootID;
  };

}

#endif /* _SINGLE_CONTACT_H_ */
