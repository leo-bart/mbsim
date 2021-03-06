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

#ifndef _CONTACT_H_
#define _CONTACT_H_

#include <mbsim/links/single_contact.h>
#include <map>

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
  class Contact : public Link {
    public:
      /*!
       * \brief constructor
       * \param name of contact
       */
      Contact(const std::string &name = "");

      /**
       * \brief destructor
       */
      virtual ~Contact();

      /* INHERITED INTERFACE OF LINKINTERFACE */
      virtual void updatewb();
      virtual void updateW(int i = 0);
      virtual void updateV(int i = 0);
      virtual void updateh(int i = 0);
      virtual void updateg();
      virtual void updategd();
      virtual void updateStopVector();
      virtual void updateJacobians(int j = 0);
      /***************************************************/

      /* INHERITED INTERFACE OF LINK */
      virtual void updateWRef(const fmatvec::Mat &ref, int j = 0);
      virtual void updateVRef(const fmatvec::Mat &ref, int j = 0);
      virtual void updatehRef(const fmatvec::Vec &hRef, int j = 0);
      virtual void updaterRef(const fmatvec::Vec &hRef, int j = 0);
      virtual void updatewbRef(const fmatvec::Vec &ref);
      virtual void updatelaRef(const fmatvec::Vec& ref);
      virtual void updateLaRef(const fmatvec::Vec& ref);
      virtual void updategRef(const fmatvec::Vec& ref);
      virtual void updategdRef(const fmatvec::Vec& ref);
      virtual void updateresRef(const fmatvec::Vec& ref);
      virtual void updaterFactorRef(const fmatvec::Vec &ref);
      virtual void updatesvRef(const fmatvec::Vec &ref);
      virtual void updatejsvRef(const fmatvec::VecInt &ref);
      virtual void updateLinkStatusRef(const fmatvec::VecInt &LinkStatusParent);
      virtual void updateLinkStatusRegRef(const fmatvec::VecInt &LinkStatusRegParent);
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
      virtual void setGeneralizedForceTolerance(double tol);
      virtual void setGeneralizedImpulseTolerance(double tol);
      virtual void setGeneralizedRelativePositionTolerance(double tol);
      virtual void setGeneralizedRelativeVelocityTolerance(double tol);
      virtual void setGeneralizedRelativeAccelerationTolerance(double tol);
      virtual void setrMax(double rMax_);
      virtual void setLinkStatusInd(int LinkStatusInd_);
      virtual void setLinkStatusRegInd(int LinkStatusRegInd_);
      virtual void setsvInd(int svInd_);
      virtual void setlaInd(int laInd_);
      virtual void setgInd(int gInd_);
      virtual void setgdInd(int gdInd_);
      virtual void setrFactorInd(int rFactorInd_);
      virtual void LinearImpactEstimation(double t, fmatvec::Vec &gInActive_, fmatvec::Vec &gdInActive_, int *IndInActive_, fmatvec::Vec &gAct_, int *IndActive_);
      virtual void SizeLinearImpactEstimation(int *sizeInActive_, int *sizeActive_);
      virtual void updatecorrRef(const fmatvec::Vec& ref);
      virtual void updatecorr(int j);
      virtual void calccorrSize(int j);
      virtual void setcorrInd(int corrInd_);
      virtual void checkRoot();
      virtual void updateGeneralizedNormalForce();
      /***************************************************/

      /* INHERITED INTERFACE OF ELEMENT */
      virtual std::string getType() const {
        return "Contact";
      }
      virtual void plot();
      virtual void setDynamicSystemSolver(DynamicSystemSolver *sys);
      /***************************************************/

      void resetUpToDate();

      /* GETTER / SETTER */

      void setNormalForceLaw(GeneralizedForceLaw *fcl_);
      GeneralizedForceLaw * getNormalForceLaw() const { return fcl; }
      void setNormalImpactLaw(GeneralizedImpactLaw *fnil_);
      void setTangentialForceLaw(FrictionForceLaw *fdf_); 
      void setTangentialImpactLaw(FrictionImpactLaw *ftil_);
      void setContactKinematics(ContactKinematics* ck, size_t index = 0) {
        assert(index < contactKinematics.size());
        contactKinematics[index] = ck;
      }
      ContactKinematics* getContactKinematics(size_t index = 0) const {
        assert(index < contactKinematics.size());
        return contactKinematics[index];
      }

      ContactKinematics* findContactKinematics(std::string cKName) const {
        int pos = find(ckNames.begin(), ckNames.end(), cKName) - ckNames.begin();
        if (pos < static_cast<int>(ckNames.size())) {
          return contactKinematics[pos];
        }
        throw MBSimError("Name of contact Kinematics is not valid");
        return NULL;
      }

      const std::vector<std::vector<SingleContact> > & getSubcontacts() const {
        return contacts;
      }

      virtual void setPlotFeatureContactKinematics(std::string cKName, std::size_t pf, PlotFeatureStatus value);
      /***************************************************/

      /**
       * \return number of considered friction directions
       */
      virtual int getFrictionDirections();

      /*! connect two contours
       * \param contour0          first contour
       * \param contour1          second contour
       * \param contactKinematics The contact kinematics that should be used to compute the contact point.
       * \param name              Name of the contact in the output
       *
       */
      void connect(Contour *contour1, Contour* contour2, ContactKinematics* contactKinematics = 0, const std::string & name = "");

      Contour* getContour(int i, int j=0) { return contour[i][j]; }

      SingleContact& getSingleContact(int i, int j=0) { return contacts[i][j]; }

      virtual void initializeUsingXML(xercesc::DOMElement *element);

      void setSearchAllContactPoints(bool searchAllCP_) { searchAllCP = searchAllCP_; }
      void setInitialGuess(const fmatvec::VecV &zeta0_) { zeta0 = zeta0_; }

    protected:
      /**
       * \brief list of the single sub-contact(-points)
       */
      std::vector<std::vector<SingleContact> > contacts;

      /**
       * \brief list of the single contact kinematics
       */
      std::vector<ContactKinematics*> contactKinematics;

      std::vector<std::vector<Contour*> > contour;

      /*!
       * \brief names for the contact kinematics
       *
       * \todo: what really is annoying is the fact, that due to the concept of the compound contour the sub contacts can not be build when the contours are connected. Thus it is not possible before the contact kinematics are assigned (what happens in the preInit-stage) that plot featers (and everything else, like names for the plot and so on) can not be set before. Thus this properties have to be saved in a special list in the multiple contact or the things are set later on...
       */
      std::vector<std::string> ckNames;

      /*!
       * \brief plotFeatures of sub-contacts
       *
       * \todo: see remark of ckNames
       */
      std::map<std::pair<std::string, std::size_t>, PlotFeatureStatus> plotFeatureMap;

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

      bool searchAllCP;

      fmatvec::VecV zeta0;

    private:
      struct saved_references {
          std::string name1;
          std::string name2;
          std::string contourPairingName;
      };
      std::vector<saved_references> saved_ref;
  };

}

#endif /* _CONTACT_H_ */
