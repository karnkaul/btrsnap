add_library(${package_name}-lib)
add_library(${package_name}::${package_name}-lib ALIAS ${package_name}-lib)

target_compile_features(${package_name}-lib PUBLIC cxx_std_23)

target_link_libraries(${package_name}-lib PUBLIC
  klib::klib
  confy::confy
)

target_include_directories(${package_name}-lib PRIVATE
  src
)

file(GLOB_RECURSE headers LIST_DIRECTORIES false "include/*.hpp")
target_sources(${package_name}-lib PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
  ${headers}
)

configure_file(src/build_version.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/include/${package_name}/build_version.hpp")
target_sources(${package_name}-lib PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include" FILES
  "${CMAKE_CURRENT_BINARY_DIR}/include/${package_name}/build_version.hpp"
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${package_name}-lib PRIVATE
  ${sources}
)
