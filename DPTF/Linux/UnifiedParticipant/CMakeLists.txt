if (IN_SOURCE_BUILD MATCHES YES)
        include_directories(..)
        include_directories(../../../Common)
        include_directories(../ThirdParty)
        include_directories(../SharedLib)
	include_directories(../SharedLib/BasicTypesLib)
	include_directories(../SharedLib/DptfObjectsLib)
	include_directories(../SharedLib/EsifTypesLib)
	include_directories(../SharedLib/DptfTypesLib)
	include_directories(../SharedLib/ParticipantControlsLib)
	include_directories(../SharedLib/ParticipantLib)
	include_directories(../SharedLib/EventsLib)
	include_directories(../SharedLib/MessageLoggingLib)
	include_directories(../SharedLib/XmlLib)
        file(GLOB_RECURSE participant_SOURCES "*.cpp")
else ()
        include_directories(../../Sources)
        include_directories(../../../Common)
        include_directories(../../Sources/ThirdParty)
        include_directories(../../Sources/SharedLib)
	include_directories(../../Sources/SharedLib/BasicTypesLib)
	include_directories(../../Sources/SharedLib/EsifTypesLib)
	include_directories(../../Sources/SharedLib/DptfTypesLib)
	include_directories(../../Sources/SharedLib/ParticipantControlsLib)
	include_directories(../../Sources/SharedLib/ParticipantLib)
	include_directories(../../Sources/SharedLib/EventsLib)
	include_directories(../../Sources/SharedLib/MessageLoggingLib)
	include_directories(../../Sources/SharedLib/XmlLib)
	include_directories(../../Sources/SharedLib/DptfObjectsLib)
        file(GLOB_RECURSE participant_SOURCES "../../Sources/UnifiedParticipant/*.cpp")
endif()

add_library(${UNIFIED_PARTICIPANT} STATIC ${participant_SOURCES})

set_target_properties(${UNIFIED_PARTICIPANT} PROPERTIES PREFIX "")

target_link_libraries(${UNIFIED_PARTICIPANT} ${SHARED_LIB} ${BASIC_TYPES_LIB}
	${ESIF_TYPES_LIB} ${DPTF_TYPES_LIB} ${DPTF_OBJECTS_LIB} ${XML_LIB}
	${PARTICIPANT_CONTROLS_LIB} ${MESSAGE_LOGGING_LIB} ${PARTICIPANT_LIB} ${EVENTS_LIB}) 

if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${UNIFIED_PARTICIPANT}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${UNIFIED_PARTICIPANT}.a.debug
	)
endif()