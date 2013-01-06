INCLUDE(../../cmakemodules/AssureCMakeRootFile.cmake) # Avoid user mistake in CMake source directory

#-----------------------------------------------------------------
# CMake file for the MRPT application:  holonomic-navigator-demo
#
#  Run with "cmake ." at the root directory
#-----------------------------------------------------------------

PROJECT(robotic-arm-kinematics)

IF(wxWidgets_FOUND)

	# ---------------------------------------------
	# TARGET:
	# ---------------------------------------------
	SET(SRCS
	robotic_arm_kinematicsApp.cpp robotic_arm_kinematicsApp.h
	robotic_arm_kinematicsMain.cpp robotic_arm_kinematicsMain.h
	PanelDOF.cpp PanelDOF.h
	CAboutBox.cpp  CAboutBox.h
	MyGLCanvas.cpp MyGLCanvas.h
	)

	IF(WIN32)
		SET(SRCS ${SRCS} resource.rc)
	ENDIF(WIN32)

	INCLUDE_DIRECTORIES("${MRPT_SOURCE_DIR}/otherlibs/wxThings/")

	SET(TMP_TARGET_NAME "robotic-arm-kinematics")

	# Define the executable target:
	ADD_EXECUTABLE(${TMP_TARGET_NAME} WIN32 ${SRCS})

	SET_TARGET_PROPERTIES(${TMP_TARGET_NAME} PROPERTIES PROJECT_LABEL "(APP) ${TMP_TARGET_NAME}")

	# Add the required libraries for linking:
	TARGET_LINK_LIBRARIES(${TMP_TARGET_NAME} ${wxWidgets_LIBRARIES} )
	TARGET_LINK_LIBRARIES(${TMP_TARGET_NAME} ${MRPT_LINKER_LIBS})

	TARGET_LINK_LIBRARIES(${TMP_TARGET_NAME} mrpt-kinematics)
	
	TARGET_LINK_LIBRARIES(${TMP_TARGET_NAME} mrptwxthings)
	ADD_DEPENDENCIES(${TMP_TARGET_NAME} mrptwxthings)

	# Dependencies on MRPT libraries:
	#  Just mention the top-level dependency, the rest will be detected automatically, 
	#  and all the needed #include<> dirs added (see the script DeclareAppDependencies.cmake for further details)
	DeclareAppDependencies(${TMP_TARGET_NAME} mrpt-kinematics mrpt-gui)

	# ----------------------------------------------------------------------------
	#   				INSTALL
	# ----------------------------------------------------------------------------
	INSTALL(TARGETS ${TMP_TARGET_NAME}
	    RUNTIME DESTINATION ${mrpt_apps_INSTALL_PREFIX}bin
	    LIBRARY DESTINATION ${mrpt_apps_INSTALL_PREFIX}lib${LIB_SUFFIX}
	    ARCHIVE DESTINATION ${mrpt_apps_INSTALL_PREFIX}lib${LIB_SUFFIX}
		)
			
ENDIF(wxWidgets_FOUND)
