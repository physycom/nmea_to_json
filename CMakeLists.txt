cmake_minimum_required (VERSION 3.1)
project (nmea_to_json)
set (CMAKE_CXX_STANDARD 11)

### Set output directories on a per-configuration base
# Single configuration
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY $ENV{HOME}/bin )
# Multi configuration
foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
    string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
    if( OUTPUTCONFIG STREQUAL "RELEASE" )
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} $ENV{HOME}/bin )
    else()
    set( CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_SOURCE_DIR}/bin/${OUTPUTCONFIG} )
    endif()
endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )

set(JSONCONS_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/jsoncons/src)
if(MSVC)
  add_definitions(-D_SCL_SECURE_NO_WARNINGS)
  add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

find_package(Boost REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${JSONCONS_INCLUDE_DIR})

add_executable(nmea_to_json ${CMAKE_SOURCE_DIR}/src/nmea_to_json.cpp)
install (TARGETS nmea_to_json DESTINATION ${CMAKE_SOURCE_DIR}/bin)
