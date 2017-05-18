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
find_package(YARP 2.3.69.9 REQUIRED)

# yarpframecontroller
add_subdirectory(yarpframecontroller)