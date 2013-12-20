if (IN_SOURCE_BUILD MATCHES YES)
	file(GLOB_RECURSE manager_SOURCES "*.cpp")
else ()
	file(GLOB_RECURSE manager_SOURCES "../../../Sources/Manager/Manager/*.cpp")
endif()	

find_package(Threads REQUIRED)

add_library(${MANAGER} MODULE ${manager_SOURCES})

set_target_properties(${MANAGER} PROPERTIES PREFIX "")

target_link_libraries(${MANAGER} ${SHARED_LIB} ${UNIFIED_PARTICIPANT} rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
