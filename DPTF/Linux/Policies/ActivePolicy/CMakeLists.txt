set(ACTIVE_POLICY "DptfPolicyActive")

if (IN_SOURCE_BUILD MATCHES YES)
        file(GLOB_RECURSE active_policy_SOURCES "*.cpp")
else ()
	file(GLOB_RECURSE active_policy_SOURCES "../../../Sources/Policies/ActivePolicy/*.cpp")
endif()

add_library(${ACTIVE_POLICY} MODULE ${active_policy_SOURCES})

set_target_properties(${ACTIVE_POLICY} PROPERTIES PREFIX "")

target_link_libraries(${ACTIVE_POLICY} ${POLICY_LIB} ${SHARED_LIB})
