# Copyright: (C) 2017 Fondazione Istituto Italiano di Tecnologia
# Authors: Andrea Ruzzenenti
# CopyPolicy: Released under the terms of the LGPLv2.1 or later, see LGPL.TXT


add_executable(yarpframecontroller yarpframecontroller.cpp ParamParser.h)
target_include_directories(yarpframecontroller PUBLIC ${YARP_INCLUDE_DIRS})
target_link_libraries(yarpframecontroller ${YARP_LIBRARIES})

install(TARGETS yarpframecontroller
        DESTINATION ${CMAKE_INSTALL_BINDIR})

