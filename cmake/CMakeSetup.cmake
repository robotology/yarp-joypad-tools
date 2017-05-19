################################################################################
#                                                                              #
# Copyright (C) 2017 Fondazione Istitito Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# @authors: Silvio Traversaro <silvio.traversaro@iit.it>

### General CMake YARP setup
# See https://github.com/robotology/how-to-export-cpp-library/

# Defines the CMAKE_INSTALL_LIBDIR, CMAKE_INSTALL_BINDIR and many other useful macros
# See https://cmake.org/cmake/help/latest/module/GNUInstallDirs.html
include(GNUInstallDirs)

# Create build artifacts in ${build}/bin for binaries and ${build}/lib for libraries
# This simplifies the use of this software directly from the build directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")

# Add uninstall target
include(AddUninstallTarget)

# Add C++11 support for a specific part of iDynTree
include(CheckCXXCompilerFlag)
unset(CXX11_FLAGS)
check_cxx_compiler_flag("-std=c++11" CXX_HAS_STD_CXX11)
check_cxx_compiler_flag("-std=c++0x" CXX_HAS_STD_CXX0X)
if(CXX_HAS_STD_CXX11)
  set(CXX11_FLAGS "-std=c++11")
elseif(CXX_HAS_STD_CXX0X)
  set(CXX11_FLAGS "-std=c++0x")
endif()

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED 11)
if(NOT CMAKE_MINIMUM_REQUIRED_VERSION VERSION_LESS 3.1)
  message(AUTHOR_WARNING "CMAKE_MINIMUM_REQUIRED_VERSION is now ${CMAKE_MINIMUM_REQUIRED_VERSION}. This check can be removed.")
endif()
if(${CMAKE_VERSION} VERSION_LESS 3.1)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX11_FLAGS}")
endif()
