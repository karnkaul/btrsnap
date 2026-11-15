add_library(${package_name}-lib)
add_library(${package_name}::${package_name}-lib ALIAS ${package_name}-lib)

target_compile_features(${package_name}-lib PUBLIC cxx_std_23)

target_link_libraries(${package_name}-lib
  PUBLIC
  klib::klib
  djson::djson

  PRIVATE
  btrfsutil
)

file(GLOB_RECURSE headers LIST_DIRECTORIES false "include/*.hpp")
target_sources(${package_name}-lib PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
  ${headers}
)

file(GLOB_RECURSE modules LIST_DIRECTORIES false "public/*.cppm" "private/*.cppm")
file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")

set(version_module public/build_version.cppm)
configure_file(public/build_version.cppm.in ${version_module})
target_sources(${package_name}-lib PUBLIC FILE_SET CXX_MODULES 
  BASE_DIRS 
  "${CMAKE_CURRENT_SOURCE_DIR}"
  "${CMAKE_CURRENT_BINARY_DIR}"
  
  FILES
  "${CMAKE_CURRENT_BINARY_DIR}/${version_module}"
  ${modules}
)

target_sources(${package_name}-lib PRIVATE
  ${sources}
)
