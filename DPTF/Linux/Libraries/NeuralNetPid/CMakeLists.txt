if (IN_SOURCE_BUILD MATCHES YES)
        include_directories(../)
        file(GLOB_RECURSE neural_net_pid_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        file(GLOB_RECURSE neural_net_pid_lib_SOURCES "../../../Sources/Libraries/NeuralNetPid/*.cpp")
endif()

add_library(${NEURALNETPID_LIB} STATIC ${neural_net_pid_lib_SOURCES})

set_target_properties(${NEURALNETPID_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${NEURALNETPID_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${NEURALNETPID_LIB}.a.debug
	)
endif()
