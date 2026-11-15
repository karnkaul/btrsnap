add_executable(${PROJECT_NAME}-tests)

target_link_libraries(${PROJECT_NAME}-tests PRIVATE
  klib::klib-test-main
  ${package_name}::${package_name}-lib
)

file(GLOB_RECURSE modules LIST_DIRECTORIES false "*.cppm")
file(GLOB_RECURSE sources LIST_DIRECTORIES false "*.cpp")

target_sources(${PROJECT_NAME}-tests PRIVATE FILE_SET CXX_MODULES FILES ${modules})
target_sources(${PROJECT_NAME}-tests PRIVATE ${sources})

add_test(NAME ${PROJECT_NAME}-tests COMMAND ${PROJECT_NAME}-tests)

