cmake_minimum_required(VERSION 3.12)
project(simple-sg)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/includes/*.h"
    "${CMAKE_SOURCE_DIR}/includes/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/includes")
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        "${CMAKE_SOURCE_DIR}/includes/md4c.lib"
        "${CMAKE_SOURCE_DIR}/includes/md4c-html.lib"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
)

source_group(TREE "${CMAKE_SOURCE_DIR}" FILES ${SOURCES} ${HEADERS})

if(CMAKE_GENERATOR MATCHES "Visual Studio")
  set_property(DIRECTORY "${CMAKE_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT "${PROJECT_NAME}")
endif()
