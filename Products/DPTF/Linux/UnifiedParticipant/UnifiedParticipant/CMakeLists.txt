if (IN_SOURCE_BUILD MATCHES YES)
    file(GLOB_RECURSE participant_SOURCES "*.cpp")
else ()
    file(GLOB_RECURSE participant_SOURCES "../../../Sources/UnifiedParticipant/UnifiedParticipant/*.cpp")
endif()

add_library(${UNIFIED_PARTICIPANT} STATIC ${participant_SOURCES})

set_target_properties(${UNIFIED_PARTICIPANT} PROPERTIES PREFIX "")

target_link_libraries(${UNIFIED_PARTICIPANT} ${SHARED_LIB}) 
