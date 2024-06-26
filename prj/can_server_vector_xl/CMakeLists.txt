cmake_minimum_required(VERSION 2.8.1)

#message(STATUS "(. ${.})")

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING
      "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
      FORCE)
endif(NOT CMAKE_BUILD_TYPE)

project(CAN_SERVER_VECTOR_XL)


if(WIN32)
  add_definitions(-DSYSTEM_PC_VC)
endif(WIN32)

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
  add_definitions(
    -DHAVE_STRUCT_TIMESPEC=1)

  include_directories(
    ../../3rd/lib/pthread/include
    "C:/Users/Public/Documents/Vector XL Driver Library/bin"
    ../../3rd/src/yasper/)

  find_package(Threads REQUIRED)
  link_directories(
    "../../3rd/lib/pthread/lib"
    "C:/Users/Public/Documents/Vector XL Driver Library/bin"
    )
  set(ISOAGLIB_ADDITIONAL_LIBRARIES odbc32 odbccp32 winmm pthreadVC2 ws2_32 vxlapi)
else()
  message( FATAL_ERROR "This can server variant can't be build on non Windows systems.")
endif(WIN32)

add_executable(
  CAN-Server_vector_xl
  ../../src/can_server_common.cpp
  ../../src/can_server.cpp
  ../../src/can_filtering.cpp
  ../../src_devices/vector_xl/can_device_vector_xl.cpp)

target_link_libraries(CAN-Server_vector_xl ${ISOAGLIB_ADDITIONAL_LIBRARIES})
