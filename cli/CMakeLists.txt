add_executable(${package_name})

target_link_libraries(${package_name} PRIVATE
  ${package_name}::${package_name}-lib
  clap::clap
)

target_sources(${package_name} PRIVATE FILE_SET CXX_MODULES FILES src/cli.cppm)
target_sources(${package_name} PRIVATE src/main.cpp src/cli.cpp)

if (BTRSNAP_INSTALL)
  include(GNUInstallDirs)
  message(STATUS "setting up btrsnap install")
  install(TARGETS ${package_name}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
  )
endif()
