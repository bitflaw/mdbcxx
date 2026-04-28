cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(mdbcxx
  #VERSION 0.0.0
  LANGUAGES CXX
)

#-------------------------PkgConfig----------------------------#

find_package(PkgConfig REQUIRED)
pkg_check_modules(MARIADB REQUIRED IMPORTED_TARGET libmariadb)

#-------------------------SET GLOBAL CONSTANTS----------------------------#

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_FLAGS_DEBUG "-g")
set (CMAKE_CXX_FLAGS_RELEASE "-O3")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall -Wextra -Wpedantic)

#-------------------------SET OPTIONS------------------------------------#
option(BUILD_TESTS "Build Tests" OFF)

#-------------------------SET AND BUILD TESTS----------------------------#

if(BUILD_TESTS)
  find_package(Catch2 3 REQUIRED)

  set(TEST_SRCS
    tests/connection.cpp
    tests/params.cpp
    tests/transaction.cpp
  )

  add_executable(tests ${TEST_SRCS})
  target_include_directories(tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/mdbcxx)
  target_link_libraries(tests PRIVATE Catch2::Catch2WithMain mdbcxx_shared PkgConfig::MARIADB)

endif()

#-------------------------SET AND BUILD SOURCES AND LIBS----------------------------#

set(SOURCES
  src/field.cpp
  src/field_metadata.cpp
  src/result.cpp
  src/result_metadata.cpp
  src/row.cpp
  src/connection.cpp
  src/transaction.cpp
  src/params.cpp
)

add_library(mdbcxx_shared SHARED ${SOURCES})
set_target_properties(mdbcxx_shared PROPERTIES OUTPUT_NAME "mdbcxx")

add_library(mdbcxx_static STATIC ${SOURCES})
set_target_properties(mdbcxx_static PROPERTIES OUTPUT_NAME "mdbcxx")

target_link_libraries(mdbcxx_shared PUBLIC PkgConfig::MARIADB)
target_link_libraries(mdbcxx_static PUBLIC PkgConfig::MARIADB)

#-------------------------CONFIG HEADERS ----------------------------#

target_include_directories(mdbcxx_shared
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/mdbcxx>
  $<INSTALL_INTERFACE:include>
)

target_include_directories(mdbcxx_static
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/mdbcxx>
  $<INSTALL_INTERFACE:include>
)

#configure_file(
#  ${CMAKE_CURRENT_SOURCE_DIR}/include/mdbcxx/version.hpp.in
#  ${CMAKE_CURRENT_SOURCE_DIR}/include/mdbcxx/version.hpp
#  @ONLY
#)


#-------------------------INSTALL TARGETS----------------------------#

install(TARGETS mdbcxx_shared mdbcxx_static
  EXPORT mdbcxx-targets
  ARCHIVE DESTINATION lib/mdbcxx
  LIBRARY DESTINATION lib/mdbcxx
)

install(DIRECTORY include/mdbcxx
  DESTINATION include
  FILES_MATCHING PATTERN "*.hpp"
)

install(EXPORT mdbcxx-targets
  FILE mdbcxx-config.cmake
  DESTINATION lib/cmake/mdbcxx
)
