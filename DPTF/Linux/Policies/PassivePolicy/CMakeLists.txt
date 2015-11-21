set(PASSIVE_POLICY "DptfPolicyPassive")

if (IN_SOURCE_BUILD MATCHES YES)
        file(GLOB_RECURSE passive_policy_SOURCES "*.cpp")
else ()
	file(GLOB_RECURSE passive_policy_SOURCES "../../../Sources/Policies/PassivePolicy/*.cpp")
endif()

add_library(${PASSIVE_POLICY} MODULE ${passive_policy_SOURCES})

set_target_properties(${PASSIVE_POLICY} PROPERTIES PREFIX "")

target_link_libraries(${PASSIVE_POLICY} ${POLICY_LIB} ${SHARED_LIB} ${BASIC_TYPES_LIB} ${ESIF_TYPES_LIB} ${DPTF_TYPES_LIB} ${DPTF_OBJECTS_LIB} ${PARTICIPANT_CONTROLS_LIB} ${MESSAGE_LOGGING_LIB} ${PARTICIPANT_LIB} ${EVENTS_LIB} ${XML_LIB})
