/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2012, MAPIR group, University of Malaga                |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */
#ifndef CMonteCarloLocalization3D_H
#define CMonteCarloLocalization3D_H

#include <mrpt/poses/CPose3DPDFParticles.h>
#include <mrpt/slam/PF_implementations_data.h>
#include <mrpt/slam/TMonteCarloLocalizationParams.h>

#include <mrpt/slam/link_pragmas.h>

namespace mrpt
{
	namespace slam
	{
		class CSensoryFrame;

		using namespace mrpt::poses;
		using namespace mrpt::slam;
		using namespace mrpt::bayes;

		/** Declares a class that represents a Probability Density Function (PDF) over a 3D pose (x,y,phi,yaw,pitch,roll), using a set of weighted samples.
		 *
		 *  This class also implements particle filtering for robot localization. See the MRPT
		 *   application "app/pf-localization" for an example of usage.
		 *
		 * \sa CMonteCarloLocalization2D, CPose2D, CPosePDF, CPoseGaussianPDF, CParticleFilterCapable
		 * \ingroup mrpt_slam_grp 
		 */
		class SLAM_IMPEXP CMonteCarloLocalization3D :
			public CPose3DPDFParticles,
			public PF_implementation<CPose3D,CMonteCarloLocalization3D>
		{
			//template <class PARTICLE_TYPE, class MYSELF> friend class PF_implementation;

		public:
			TMonteCarloLocalizationParams	options; //!< MCL parameters

			/** Constructor
			  * \param M The number of m_particles.
			  */
			CMonteCarloLocalization3D( size_t M = 1 );

			/** Destructor */
			virtual ~CMonteCarloLocalization3D();

			 /** Update the m_particles, predicting the posterior of robot pose and map after a movement command.
			  *  This method has additional configuration parameters in "options".
			  *  Performs the update stage of the RBPF, using the sensed CSensoryFrame:
			  *
			  *   \param action This is a pointer to CActionCollection, containing the pose change the robot has been commanded.
			  *   \param observation This must be a pointer to a CSensoryFrame object, with robot sensed observations.
			  *
			  * \sa options
			  */
			void  prediction_and_update_pfStandardProposal(
				const mrpt::slam::CActionCollection	* action,
				const mrpt::slam::CSensoryFrame		* observation,
				const bayes::CParticleFilter::TParticleFilterOptions &PF_options );

			 /** Update the m_particles, predicting the posterior of robot pose and map after a movement command.
			  *  This method has additional configuration parameters in "options".
			  *  Performs the update stage of the RBPF, using the sensed CSensoryFrame:
			  *
			  *   \param Action This is a pointer to CActionCollection, containing the pose change the robot has been commanded.
			  *   \param observation This must be a pointer to a CSensoryFrame object, with robot sensed observations.
			  *
			  * \sa options
			  */
			void  prediction_and_update_pfAuxiliaryPFStandard(
				const mrpt::slam::CActionCollection	* action,
				const mrpt::slam::CSensoryFrame		* observation,
				const bayes::CParticleFilter::TParticleFilterOptions &PF_options );

			 /** Update the m_particles, predicting the posterior of robot pose and map after a movement command.
			  *  This method has additional configuration parameters in "options".
			  *  Performs the update stage of the RBPF, using the sensed CSensoryFrame:
			  *
			  *   \param Action This is a pointer to CActionCollection, containing the pose change the robot has been commanded.
			  *   \param observation This must be a pointer to a CSensoryFrame object, with robot sensed observations.
			  *
			  * \sa options
			  */
			void  prediction_and_update_pfAuxiliaryPFOptimal(
				const mrpt::slam::CActionCollection	* action,
				const mrpt::slam::CSensoryFrame		* observation,
				const bayes::CParticleFilter::TParticleFilterOptions &PF_options );

		//protected:
			/** \name Virtual methods that the PF_implementations assume exist.
			    @{ */
			/** Return a pointer to the last robot pose in the i'th particle (or NULL if it's a path and it's empty). */
			const TPose3D * getLastPose(const size_t i) const;

			void PF_SLAM_implementation_custom_update_particle_with_new_pose(
				CParticleDataContent *particleData,
				const TPose3D &newPose) const;

			// We'll redefine this one:
			void PF_SLAM_implementation_replaceByNewParticleSet(
				CParticleList	&old_particles,
				const std::vector<TPose3D>	&newParticles,
				const vector_double		&newParticlesWeight,
				const std::vector<size_t>	&newParticlesDerivedFromIdx )  const;

			/** Evaluate the observation likelihood for one particle at a given location */
			double PF_SLAM_computeObservationLikelihoodForParticle(
				const CParticleFilter::TParticleFilterOptions	&PF_options,
				const size_t			particleIndexForMap,
				const CSensoryFrame		&observation,
				const CPose3D			&x ) const;
			/** @} */


		}; // End of class def.

	} // End of namespace
} // End of namespace

#endif
