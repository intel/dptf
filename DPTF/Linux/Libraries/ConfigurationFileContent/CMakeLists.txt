if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../Common)
        file(GLOB_RECURSE configuration_file_content_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../Sources/Common)
        file(GLOB_RECURSE configuration_file_content_lib_SOURCES "../../../Sources/Libraries/ConfigurationFileContent/*.cpp")
endif()

add_library(${CONFIGURATION_FILE_CONTENT_LIB} STATIC ${configuration_file_content_lib_SOURCES})

set_target_properties(${CONFIGURATION_FILE_CONTENT_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
    add_custom_command(TARGET ${CONFIGURATION_FILE_CONTENT_LIB}
        POST_BUILD
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a.debug
        COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a
        BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${CONFIGURATION_FILE_CONTENT_LIB}.a.debug
    )
endif()