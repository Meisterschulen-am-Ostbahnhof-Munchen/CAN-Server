cmake_minimum_required(VERSION 2.8.1)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

project(CAN_SERVER_KVASER)


set(ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS "-DDEBUG_CANSERVER=0")
set(ISOAGLIB_ADDITIONAL_DEBUG_DEFINITIONS   "-DDEBUG_CANSERVER=1")

set(CMAKE_CXX_FLAGS_DEBUG          "${CMAKE_CXX_FLAGS_DEBUG} ${ISOAGLIB_ADDITIONAL_DEBUG_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")

include_directories(
  ../../src
  ../../3rd/src/yasper/)

if(WIN32)
  include_directories(
    ../../3rd/lib/pthread/include
    ../../3rd/lib/kvaser/windows/inc)

  find_package(Threads REQUIRED)
  link_directories(
    ../../3rd/lib/pthread/lib
    ../../3rd/lib/kvaser/windows/lib)
  set(ISOAGLIB_ADDITIONAL_LIBRARIES winmm pthreadVC2 ws2_32 canlib32)
else()
  include_directories(
    /usr/include)
  find_library(CANLIB_LIBRARY canlib HINTS /usr/lib/)
  set(ISOAGLIB_ADDITIONAL_LIBRARIES pthread ${CANLIB_LIBRARY})
endif(WIN32)

add_executable(
  CAN-Server_kvaser
  ../../src/can_server_common.cpp
  ../../src/can_server.cpp
  ../../src/can_filtering.cpp
  ../../src_devices/kvaser/can_device_kvaser.cpp)

target_link_libraries(CAN-Server_kvaser ${ISOAGLIB_ADDITIONAL_LIBRARIES})
