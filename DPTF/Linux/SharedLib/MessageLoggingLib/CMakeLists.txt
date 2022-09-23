if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../../../Common)
		include_directories(../../SharedLib/BasicTypesLib)
		include_directories(../../SharedLib/EventsLib)
        file(GLOB_RECURSE message_logging_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../../Common)
		include_directories(../../../Sources/SharedLib/BasicTypesLib)
		include_directories(../../../Sources/SharedLib/EventsLib)
        file(GLOB_RECURSE message_logging_lib_SOURCES "../../../Sources/SharedLib/MessageLoggingLib/*.cpp")
endif()

add_library(${MESSAGE_LOGGING_LIB} STATIC ${message_logging_lib_SOURCES})

set_target_properties(${MESSAGE_LOGGING_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${MESSAGE_LOGGING_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MESSAGE_LOGGING_LIB}.a.debug
	)
endif()