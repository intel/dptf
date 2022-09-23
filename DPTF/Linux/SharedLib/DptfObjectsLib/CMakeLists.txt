if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../../../Common)
        include_directories(../../ThirdParty)
		include_directories(../../Sources/SharedLib)
		include_directories(../../Sources/SharedLib/BasicTypesLib)
		include_directories(../../Sources/SharedLib/ParticipantControlsLib)
		include_directories(../../Sources/SharedLib/ParticipantLib)
		include_directories(../../Sources/SharedLib/XmlLib)
		include_directories(../../Sources/SharedLib/DptfTypesLib)
        file(GLOB_RECURSE dptf_objects_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../../Common)
        include_directories(../../../Sources/ThirdParty)
		include_directories(../../../Sources/SharedLib)
		include_directories(../../../Sources/SharedLib/BasicTypesLib)
		include_directories(../../../Sources/SharedLib/ParticipantControlsLib)
		include_directories(../../../Sources/SharedLib/ParticipantLib)
		include_directories(../../../Sources/SharedLib/XmlLib)
		include_directories(../../../Sources/SharedLib/DptfTypesLib)
        file(GLOB_RECURSE dptf_objects_lib_SOURCES "../../../Sources/SharedLib/DptfObjectsLib/*.cpp")
endif()

add_library(${DPTF_OBJECTS_LIB} STATIC ${dptf_objects_lib_SOURCES})

set_target_properties(${DPTF_OBJECTS_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${DPTF_OBJECTS_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${DPTF_OBJECTS_LIB}.a.debug
	)
endif()
