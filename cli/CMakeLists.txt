add_executable(${package_name})

target_link_libraries(${package_name} PRIVATE
  ${package_name}::${package_name}-lib
  clap::clap
)

target_include_directories(${package_name} PRIVATE
  src
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${package_name} PRIVATE
  ${sources}
)
