cmake_minimum_required(VERSION 3.21)

project(FPSUnlocker)

set(CMAKE_CXX_STANDARD 20)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCES})

add_subdirectory(
    $ENV{GEODE_SDK}
    ${CMAKE_CURRENT_BINARY_DIR}/geode
)

setup_geode_mod(${PROJECT_NAME})

if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE c++)
endif()
