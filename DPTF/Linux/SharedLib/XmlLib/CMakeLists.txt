if (IN_SOURCE_BUILD MATCHES YES)                                                                                                                                                                                                                                        
        include_directories(../)
        include_directories(../../../../Common)
        include_directories(../../ThirdParty)
		include_directories(../../SharedLib/BasicTypesLib)
        file(GLOB_RECURSE xml_lib_SOURCES "*.cpp")
else ()
        include_directories(../../../Sources)
        include_directories(../../../../Common)
        include_directories(../../../Sources/ThirdParty)
		include_directories(../../../Sources/SharedLib/BasicTypesLib)
        file(GLOB_RECURSE xml_lib_SOURCES "../../../Sources/SharedLib/XmlLib/*.cpp")
endif()

add_library(${XML_LIB} STATIC ${xml_lib_SOURCES})

set_target_properties(${XML_LIB} PROPERTIES PREFIX "")
if (CMAKE_BUILD_TYPE MATCHES Release AND NOT CHROMIUM_BUILD MATCHES YES)
	add_custom_command(TARGET ${XML_LIB}
		POST_BUILD
		COMMAND cp ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a.debug
		COMMAND ${CMAKE_STRIP} --strip-debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a
		COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a.debug ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a
		BYPRODUCTS ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${XML_LIB}.a.debug
	)
endif()