if (IN_SOURCE_BUILD MATCHES YES)
        include_directories(..)
        include_directories(../EsifSdk)
        include_directories(../ThirdParty)
        include_directories(../SharedLib)
        file(GLOB_RECURSE participant_SOURCES "*.cpp")
else ()
        include_directories(../../Sources)
        include_directories(../../Sources/EsifSdk)
        include_directories(../../Sources/ThirdParty)
        include_directories(../../Sources/SharedLib)
        file(GLOB_RECURSE participant_SOURCES "../../Sources/UnifiedParticipant/*.cpp")
endif()

add_library(${UNIFIED_PARTICIPANT} STATIC ${participant_SOURCES})

set_target_properties(${UNIFIED_PARTICIPANT} PROPERTIES PREFIX "")

target_link_libraries(${UNIFIED_PARTICIPANT} ${SHARED_LIB}) 
