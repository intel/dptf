if (IN_SOURCE_BUILD MATCHES YES)
	file(GLOB_RECURSE policy_lib_SOURCES "*.cpp")
else ()
	file(GLOB_RECURSE policy_lib_SOURCES "../../../Sources/PolicyLib/PolicyLib/*.cpp")
endif()

add_library(${POLICY_LIB} STATIC ${policy_lib_SOURCES})

set_target_properties(${POLICY_LIB} PROPERTIES PREFIX "")