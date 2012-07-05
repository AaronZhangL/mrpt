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
#ifndef CAbstractReactiveNavigationSystem_H
#define CAbstractReactiveNavigationSystem_H

#include <mrpt/maps.h>
#include <mrpt/poses.h>

#include <mrpt/reactivenav/link_pragmas.h>


#include <cstdarg>

namespace mrpt
{
  namespace reactivenav
  {
	using namespace mrpt;
	using namespace mrpt::slam;
	using namespace mrpt::poses;

	/** The pure virtual class that a user of CAbstractReactiveNavigationSystem-derived classes must implement in order to allow the navigator sense the world and send motion commands to the robot.
	  *
	  *  The user must define a new class derived from CReactiveInterfaceImplementation and reimplement
	  *   all pure virtual and the desired virtual methods according to the documentation in this class.
	  *
	  * \sa CReactiveNavigationSystem, CAbstractReactiveNavigationSystem
	  *  \ingroup mrpt_reactivenav_grp
	  */
	class REACTIVENAV_IMPEXP CReactiveInterfaceImplementation
	{
	public:
		/** Get the current pose and speeds of the robot.
		 *   \param curPose Current robot pose.
		 *   \param curV Current linear speed, in meters per second.
		 *	 \param curW Current angular speed, in radians per second.
		 * \return false on any error.
		 */
		virtual bool getCurrentPoseAndSpeeds( mrpt::poses::CPose2D &curPose, float &curV, float &curW) = 0;

		/** Change the instantaneous speeds of robot.
		 *   \param v Linear speed, in meters per second.
		 *	 \param w Angular speed, in radians per second.
		 * \return false on any error.
		 */
		virtual bool changeSpeeds( float v, float w ) = 0;

		/** Stop the robot right now.
		 * \return false on any error.
		 */
		virtual bool stop() {
			return changeSpeeds(0,0);
		}

		/** Start the watchdog timer of the robot platform, if any.
		 * \param T_ms Period, in ms.
		 * \return false on any error.
		 */
		virtual bool startWatchdog(float T_ms) { return true; }

		/** Stop the watchdog timer.
		 * \return false on any error.
		 */
		virtual bool stopWatchdog() { return true; }

		/** Return the current set of obstacle points.
		  * \return false on any error.
		  */
		virtual bool senseObstacles( mrpt::slam::CSimplePointsMap 		&obstacles ) = 0;

		virtual void sendNavigationStartEvent () { std::cout << "[sendNavigationStartEvent] Not implemented by the user." << std::endl; }

		virtual void sendNavigationEndEvent() {	std::cout << "[sendNavigationEndEvent] Not implemented by the user." << std::endl; }

		virtual void sendNavigationEndDueToErrorEvent() { std::cout << "[sendNavigationEndDueToErrorEvent] Not implemented by the user." << std::endl; }

		virtual void sendWaySeemsBlockedEvent() { std::cout << "[sendWaySeemsBlockedEvent] Not implemented by the user." << std::endl; }

		virtual void notifyHeadingDirection(const double heading_dir_angle) { }

	};



	/** This is the base class for any reactive navigation system. Here is defined
	 *   the interface that users will use with derived classes where algorithms are really implemented.
	 *
	 * Changes history:
	 *		- 30/JUN/2004: Creation (JLBC)
     *		- 16/SEP/2004: Totally redesigned.
	 *		- 15/SEP/2005: Totally rewritten again, for integration into MRPT Applications Repository.
	 *		-  3/NOV/2009: All functors are finally replaced by the new virtual class CReactiveInterfaceImplementation
	 *
	 *   How to use:
	 *      - A class with callbacks must be defined by the user and provided to the constructor.
	 *      - navigationStep() must be called periodically in order to effectively run the navigation. This method will internally call the callbacks to gather sensor data and robot positioning data.
	 *
	 * \sa CReactiveNavigationSystem, CReactiveInterfaceImplementation
	 */
	class REACTIVENAV_IMPEXP CAbstractReactiveNavigationSystem : public mrpt::utils::CDebugOutputCapable
	{
	public:
		struct TNavigationParams;

		/** Constructor
		  */
		CAbstractReactiveNavigationSystem( CReactiveInterfaceImplementation &react_iterf_impl );

        /** Destructor
          */
        virtual ~CAbstractReactiveNavigationSystem()
		{
		}

		/** Cancel current navegacion.
		 */
		void cancel();

		/** Continues with suspended navigation.
		 * \sa suspend
		 */
		void resume();

		/** Evaluates the practicability of a navigation for given parameters:
		 * \returns An estimation in the range [0,1], for 0 being imposible and 1 being easy.
		 */
		virtual float  evaluate( TNavigationParams *params )=0;

		/** This method must be called periodically in order to effectively run the navigation.
		 */
		void navigationStep();

		/** Navigation request. It starts a new navigation.
		 */
		virtual void  navigate( TNavigationParams *params )=0;

		/** Changes the parameters for current navigation
		 */
		virtual void  setParams( TNavigationParams *params)=0;

		/** Suspend current navegation
		 * \sa resume
		 */
		virtual void  suspend();

		/** The struct for configuring the navigation request.
		 */
		struct TNavigationParams
		{
			/** Coordinates of desired target location.
			 */
			mrpt::poses::TPoint2D		target;

			/** The allowed distance from target in order to end the navigation.
			 */
			float           targetAllowedDistance;

			/** Whether the \a target coordinates are in global coordinates (false) or are relative to the current robot pose (true).
			 */
			bool            targetIsRelative;
		};

		/** The different states for the navigation system.
		 */
		enum TState
		{
			IDLE=0,
			NAVIGATING,
			SUSPENDED,
			NAV_ERROR
		};

		/** Returns the current navigator state.
		 */
		TState getCurrentState() const { return m_navigationState; }

	private:
		/** Last internal state of navigator:
		 */
		TState		m_lastNavigationState;

	protected:
		/** To be implemented in derived classes
		  */
		virtual void  performNavigationStep( )=0;

		/** Current internal state of navigator:
		 */
		TState		m_navigationState;

		/** Current navigation parameters:
		 */
		TNavigationParams	m_navigationParams;


		CReactiveInterfaceImplementation   &m_robot; //!< The navigator-robot interface.

	public:
		EIGEN_MAKE_ALIGNED_OPERATOR_NEW
	};
  }
}


#endif

