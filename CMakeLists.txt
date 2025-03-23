cmake_minimum_required(VERSION 3.21)
project(ulid VERSION 1.0.0 DESCRIPTION "Header-only ULID implementation" LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(ulid INTERFACE)
target_include_directories(ulid INTERFACE
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

include(GNUInstallDirs)
install(FILES include/ulid.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

install(TARGETS ulid
  EXPORT ulid-targets
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

option(ULID_ENABLE_OPENSSL "Enable using Openssl for sourcing entropy" OFF)
if (ULID_ENABLE_OPENSSL)
  add_compile_definitions(ULID_ENABLE_OPENSSL=1)
endif()

# Generate and install package configuration files
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/ulid-config-version.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/ulid-config.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/ulid-config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ulid
)

install(EXPORT ulid-targets
  FILE ulid-targets.cmake
  NAMESPACE ulid::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ulid
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/ulid-config.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/ulid-config-version.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ulid
)

option(BUILD_TESTING "Build the testing tree." OFF)
if(BUILD_TESTING)
  enable_testing()

  find_package(GTest REQUIRED)

  add_executable(ulid_test ulid_test.cpp)
  target_link_libraries(ulid_test PRIVATE ulid GTest::gtest GTest::gtest_main)


  # if(ENABLE_COVERAGE AND CMAKE_BUILD_TYPE STREQUAL "Debug")
  #   if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  #     target_compile_options(ulid_test PRIVATE --coverage)
  #     target_link_options(ulid_test PRIVATE --coverage)
  #   endif()
  # endif()

  include(GoogleTest)
  gtest_discover_tests(ulid_test)

  # add_test(NAME ulid_test COMMAND ulid_test)
  add_test(AllTestsInMain main)
  add_dependencies(ulid_test ulid)
  message("Tests Built")
endif()

option(BUILD_EXAMPLES "Build the examples." OFF)
if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
