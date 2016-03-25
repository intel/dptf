if (IN_SOURCE_BUILD MATCHES YES)
        file(GLOB_RECURSE POLICY_LIB_SOURCES "*.cpp")
else ()
        file(GLOB_RECURSE POLICY_LIB_SOURCES "../../../Sources/Policies/PolicyLib/*.cpp")
endif()

add_library(${POLICY_LIB} STATIC ${POLICY_LIB_SOURCES})

set_target_properties(${POLICY_LIB} PROPERTIES PREFIX "")
