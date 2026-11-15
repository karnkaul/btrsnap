cmake_minimum_required(VERSION 4.4)

if (CMAKE_VERSION VERSION_LESS "4.5.0")
  set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
  set(CMAKE_CXX_MODULE_STD ON)
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

set(package_name btrsnap)
project(${package_name} VERSION 0.4.1)

option(BTRSNAP_BUILD_TESTS "Build btrsnap tests" ${PROJECT_IS_TOP_LEVEL})
option(BTRSNAP_INSTALL "Setup btrsnap (CLI) install" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ext)
add_subdirectory(lib)
add_subdirectory(cli)

if (BTRSNAP_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
