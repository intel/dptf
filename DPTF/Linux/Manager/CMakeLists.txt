if (IN_SOURCE_BUILD MATCHES YES)
	include_directories(..)
	include_directories(../EsifSdk)
	include_directories(../ThirdParty)
	include_directories(../SharedLib)
	file(GLOB_RECURSE manager_SOURCES "*.cpp")
else ()
	include_directories(../../Sources)
	include_directories(../../Sources/EsifSdk)
	include_directories(../../Sources/ThirdParty)
	include_directories(../../Sources/SharedLib)
	file(GLOB_RECURSE manager_SOURCES "../../Sources/Manager/*.cpp")
endif()

find_package(Threads REQUIRED)

add_library(${MANAGER} MODULE ${manager_SOURCES})

set_target_properties(${MANAGER} PROPERTIES PREFIX "")

target_link_libraries(${MANAGER} ${SHARED_LIB} ${UNIFIED_PARTICIPANT} rt ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT})
