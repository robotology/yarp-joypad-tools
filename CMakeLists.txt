################################################################################
#                                                                              #
# Copyright (C) 2017 Fondazione Istituto Italiano di Tecnologia (IIT)          #
# All Rights Reserved.                                                         #
#                                                                              #
################################################################################

# @authors: Andrea Ruzzenenti <andrea.ruzzenenti@iit.it>

cmake_minimum_required(VERSION 2.8.12)
project(yarp-joypad-tools)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# Generic CMake setup
include(CMakeSetup)

# Find YARP
set(YARP_REQUIRED_VERSION 2.3.69.9)
find_package(YARP REQUIRED)
if(${YARP_VERSION} VERSION_LESS ${YARP_REQUIRED_VERSION})
  message(FATAL_ERROR "YARP version ${YARP_VERSION} not sufficient, at least version ${YARP_REQUIRED_VERSION} is required.")
endif()

# yarpframecontroller
add_subdirectory(yarpframecontroller)
