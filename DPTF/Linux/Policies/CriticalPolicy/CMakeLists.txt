set(CRITICAL_POLICY "DptfPolicyCritical")

if (IN_SOURCE_BUILD MATCHES YES)
        file(GLOB_RECURSE critical_policy_SOURCES "*.cpp")
else ()
	file(GLOB_RECURSE critical_policy_SOURCES "../../../Sources/Policies/CriticalPolicy/*.cpp")
endif()

add_library(${CRITICAL_POLICY} MODULE ${critical_policy_SOURCES})

set_target_properties(${CRITICAL_POLICY} PROPERTIES PREFIX "")

target_link_libraries(${CRITICAL_POLICY} ${POLICY_LIB} ${SHARED_LIB} ${BASIC_TYPES_LIB} ${ESIF_TYPES_LIB} ${DPTF_TYPES_LIB} ${DPTF_OBJECTS_LIB} ${PARTICIPANT_CONTROLS_LIB} ${MESSAGE_LOGGING_LIB} ${PARTICIPANT_LIB} ${EVENTS_LIB} ${XML_LIB})
