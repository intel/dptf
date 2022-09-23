if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../../../Common)
        include_directories(../../ThirdParty)
		include_directories(../../SharedLib)
		include_directories(../../SharedLib/BasicTypesLib)
		include_directories(../../SharedLib/XmlLib)
		include_directories(../../SharedLib/DptfTypesLib)
		include_directories(../../SharedLib/DptfObjectsLib)
		include_directories(../../SharedLib/ParticipantControlsLib)
        file(GLOB_RECURSE participant_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../../Common)
        include_directories(../../../Sources/ThirdParty)
		include_directories(../../../Sources/SharedLib)
		include_directories(../../../Sources/SharedLib/BasicTypesLib)
		include_directories(../../../Sources/SharedLib/XmlLib)
		include_directories(../../../Sources/SharedLib/DptfTypesLib)
		include_directories(../../../Sources/SharedLib/DptfObjectsLib)
		include_directories(../../../Sources/SharedLib/ParticipantControlsLib)
        file(GLOB_RECURSE participant_lib_SOURCES "../../../Sources/SharedLib/ParticipantLib/*.cpp")
endif()

add_library(${PARTICIPANT_LIB} STATIC ${participant_lib_SOURCES})

set_target_properties(${PARTICIPANT_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${PARTICIPANT_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PARTICIPANT_LIB}.a.debug
	)
endif()