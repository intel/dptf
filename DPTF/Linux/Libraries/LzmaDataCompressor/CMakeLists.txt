if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../Common)
        file(GLOB_RECURSE lzma_data_compressor_lib_CSOURCES "lzma/*.c")
        file(GLOB_RECURSE lzma_data_compressor_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../Sources/Common)
        file(GLOB_RECURSE lzma_data_compressor_lib_CSOURCES "../../../Sources/Libraries/LzmaDataCompressor/lzma/*.c")
        file(GLOB_RECURSE lzma_data_compressor_lib_SOURCES "../../../Sources/Libraries/LzmaDataCompressor/*.cpp")
endif()
add_definitions(-D_7ZIP_ST)

add_library(${LZMA_DATA_COMPRESSOR_LIB} STATIC ${lzma_data_compressor_lib_CSOURCES} ${lzma_data_compressor_lib_SOURCES})

set_target_properties(${LZMA_DATA_COMPRESSOR_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
    add_custom_command(TARGET ${LZMA_DATA_COMPRESSOR_LIB}
        POST_BUILD
        COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a.debug
        COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a
        BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${LZMA_DATA_COMPRESSOR_LIB}.a.debug
    )
endif()
