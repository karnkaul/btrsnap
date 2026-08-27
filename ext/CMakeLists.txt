file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT TARGET confy::confy)
  message(STATUS "[confy]")
  add_subdirectory(src/confy)
endif()

if(NOT TARGET klib::klib)
  message(STATUS "[klib]")
  set(KLIB_INSTALL ${DEPZIP_INSTALL})
  set(KLIB_UNIT_TEST ${DEPZIP_BUILD_TESTS})
  add_subdirectory(src/klib)
endif()

if(NOT TARGET clap::clap)
  message(STATUS "[clap]")
  add_subdirectory(src/clap)
endif()
