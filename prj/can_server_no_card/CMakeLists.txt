cmake_minimum_required(VERSION 2.8.1)

#message(STATUS "(. ${.})")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

project(CAN_SERVER_NO_CARD)


set(ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS "-DDEBUG_CANSERVER=0")
set(ISOAGLIB_ADDITIONAL_DEBUG_DEFINITIONS "-DDEBUG_CANSERVER=1")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${ISOAGLIB_ADDITIONAL_DEBUG_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")
set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} ${ISOAGLIB_ADDITIONAL_RELEASE_DEFINITIONS}")

include_directories(
  ../../src
  )

if(WIN32)
  include_directories(../../3rd/lib/pthread/include)
  include_directories(../../3rd/src/yasper/)
  find_package(Threads REQUIRED)
  link_directories("../../3rd/lib/pthread/lib")
  set(ISOAGLIB_ADDITIONAL_LIBRARIES odbc32 odbccp32 winmm pthreadVC2 ws2_32)
else()
  set(ISOAGLIB_ADDITIONAL_LIBRARIES rt pthread)
endif(WIN32)

add_executable(
  CAN-Server_no_card
  ../../src/can_server_common.cpp
  ../../src/can_server.cpp
  ../../src/can_filtering.cpp
  ../../src_devices/no_card/can_device_no_card.cpp)

target_link_libraries(CAN-Server_no_card ${ISOAGLIB_ADDITIONAL_LIBRARIES})
