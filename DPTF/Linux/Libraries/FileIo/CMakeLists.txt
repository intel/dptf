if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        file(GLOB_RECURSE file_io_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        file(GLOB_RECURSE file_io_lib_SOURCES "../../../Sources/Libraries/FileIo/*.cpp")
endif()

add_library(${FILEIO_LIB} STATIC ${file_io_lib_SOURCES})

set_target_properties(${FILEIO_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${FILEIO_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${FILEIO_LIB}.a.debug
	)
endif()