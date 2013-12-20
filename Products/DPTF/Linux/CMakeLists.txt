project(DPTF)

cmake_minimum_required(VERSION 2.8.6)

if (CHROMIUM_BUILD MATCHES YES)
	message("Building for Chromium OS...")
	set(CMAKE_C_COMPILER "x86_64-cros-linux-gnu-gcc")
	set(CMAKE_CXX_COMPILER "x86_64-cros-linux-gnu-g++")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -fPIC")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-Os")

message("GCC version : " ${CMAKE_CXX_COMPILER_VERSION})

if (BUILD_ARCH MATCHES 32bit AND CMAKE_BUILD_TYPE MATCHES Release)
        message("Building 32-bit release...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/release)

elseif (BUILD_ARCH MATCHES 64bit AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Building 64-bit debug...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/debug)

elseif (BUILD_ARCH MATCHES 32bit AND CMAKE_BUILD_TYPE MATCHES Debug)
        message("Building 32-bit debug...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x32/debug)

else ()
        message("Building 64-bit release...")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64")
        set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
        set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/x64/release)
endif()

message ("CXX Flags : " ${CMAKE_CXX_FLAGS}) 
message ("CXX Debug Flags : " ${CMAKE_CXX_FLAGS_DEBUG}) 
message ("CXX Release Flags : " ${CMAKE_CXX_FLAGS_RELEASE}) 
#message ("CXX MinSizeRel Flags : " ${CMAKE_C_FLAGS_MINSIZEREL}) 

add_definitions(-DESIF_ATTR_OS_LINUX -DESIF_ATTR_USER)

set(SHARED_LIB "DptfSharedLib")
add_subdirectory(SharedLib)

set(POLICY_LIB "DptfPolicyLib")
add_subdirectory(PolicyLib)

set(UNIFIED_PARTICIPANT "DptfParticipant")
add_subdirectory(UnifiedParticipant)

set(MANAGER "Dptf")
add_subdirectory(Manager)

add_subdirectory(Policies)
