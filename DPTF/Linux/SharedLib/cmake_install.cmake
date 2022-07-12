# Install script for directory: /home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr/local")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/BasicTypesLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/EsifTypesLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/DptfTypesLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/DptfObjectsLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/ParticipantControlsLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/ParticipantLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/EventsLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/MessageLoggingLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/XmlLib/cmake_install.cmake")
  include("/home/badttnayanavm1/repo/drivers.platform.dtt.dtt/DPTF/Linux/SharedLib/ResourceLib/cmake_install.cmake")

endif()

