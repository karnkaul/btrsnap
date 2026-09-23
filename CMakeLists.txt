cmake_minimum_required(VERSION 4.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

set(package_name btrsnap)
project(${package_name} VERSION 0.2.1)

option(BTRSNAP_BUILD_TESTS "Build btrsnap tests" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)
add_subdirectory(lib)
add_subdirectory(cli)

if (BTRSNAP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
