if (IN_SOURCE_BUILD MATCHES YES)
		include_directories(../../Libraries/ConfigurationFileContent)
        file(GLOB_RECURSE POLICY_LIB_SOURCES "*.cpp")
else ()
		include_directories(../../../Sources/Libraries/ConfigurationFileContent)
        file(GLOB_RECURSE POLICY_LIB_SOURCES "../../../Sources/Policies/PolicyLib/*.cpp")
endif()

add_library(${POLICY_LIB} STATIC ${POLICY_LIB_SOURCES})

set_target_properties(${POLICY_LIB} PROPERTIES PREFIX "")

if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${POLICY_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${POLICY_LIB}.a.debug
	)
endif()