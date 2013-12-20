if (IN_SOURCE_BUILD MATCHES YES)
        file(GLOB_RECURSE shared_lib_SOURCES "*.cpp")
else ()
        file(GLOB_RECURSE shared_lib_SOURCES "../../../Sources/SharedLib/SharedLib/*.cpp")
endif()

add_library(${SHARED_LIB} STATIC ${shared_lib_SOURCES})

set_target_properties(${SHARED_LIB} PROPERTIES PREFIX "")
