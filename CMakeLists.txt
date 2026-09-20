cmake_minimum_required(VERSION 3.20)

project(abys VERSION 0.1.0 LANGUAGES CXX)

find_package(PkgConfig REQUIRED)
pkg_check_modules(TCL REQUIRED IMPORTED_TARGET tcl>=8.6)

option(ABYS_ENABLE_TESTS "Build abys tests" ON)
option(ABYS_ENABLE_COVERAGE "Enable coverage flags" OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(ABYS_ENABLE_COVERAGE)
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-O0 -g --coverage)
    add_link_options(--coverage)
  else()
    message(WARNING "Coverage flags requested but compiler does not support gcov")
  endif()
endif()

file(GLOB_RECURSE ABYS_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

set(ABYS_CORE_SOURCES ${ABYS_SOURCES})
list(FILTER ABYS_CORE_SOURCES EXCLUDE REGEX "/src/(cli/|main[.]cpp$)")

set(ABYS_CLI_SOURCES ${ABYS_SOURCES})
list(FILTER ABYS_CLI_SOURCES INCLUDE REGEX "/src/(cli/|main[.]cpp$)")

function(abys_use_bundled_slang)
  set(_slang_src "${CMAKE_SOURCE_DIR}/third_party/slang")
  if (NOT EXISTS "${_slang_src}/CMakeLists.txt")
    message(FATAL_ERROR
      "Bundled slang not found at ${_slang_src}\n"
      "Run:\n"
      "  git submodule update --init --recursive\n"
      "or configure with prebuilt slang via -Dslang_DIR=/path/to/slang/lib/cmake")
  endif()
  add_subdirectory("${_slang_src}" EXCLUDE_FROM_ALL)
  set(ABYS_SLANG_TARGET slang::slang PARENT_SCOPE)
endfunction()

option(ABYS_USE_BUNDLED_SLANG "Use bundled slang submodule" OFF)

if (ABYS_USE_BUNDLED_SLANG)
  abys_use_bundled_slang()
else()
  find_package(slang CONFIG QUIET)
  if (slang_FOUND)
    set(ABYS_SLANG_TARGET slang::slang)
  else()
    message(STATUS "slang package not found; trying bundled submodule")
    abys_use_bundled_slang()
  endif()
endif()

add_subdirectory(lib)

add_library(abys_lib ${ABYS_CORE_SOURCES})
set_target_properties(abys_lib PROPERTIES OUTPUT_NAME abys)

target_include_directories(abys_lib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(abys_lib PUBLIC cxx_std_20)

target_compile_options(abys_lib PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(abys_lib PRIVATE ${ABYS_SLANG_TARGET} boop::fast)
target_compile_definitions(abys_lib PRIVATE ABYS_HAVE_SLANG=1)

add_executable(abys ${ABYS_CLI_SOURCES})

target_link_libraries(abys PRIVATE abys_lib PkgConfig::TCL)

if(ABYS_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
