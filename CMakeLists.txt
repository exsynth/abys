cmake_minimum_required(VERSION 3.20)

project(abys VERSION 0.1.0 LANGUAGES CXX)

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

set(ABYS_CORE_SOURCES
  src/infra/diagnostics.cpp
  src/version.cpp
  src/frontend/slang.cpp
  src/frontend/slang_expr_lowering.cpp
  src/frontend/slang_lowering_internal.cpp
  src/frontend/slang_pragma.cpp
  src/frontend/slang_stmt_lowering.cpp
  src/frontend/slang_lowering.cpp
  src/ir/expr_builder.cpp
  src/ir/stmt_builder.cpp
  src/ir/tig_builder.cpp
  src/ir/tig_dumper.cpp
)

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

add_library(abys_core ${ABYS_CORE_SOURCES})

target_include_directories(abys_core
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

target_compile_features(abys_core PUBLIC cxx_std_20)

target_compile_options(abys_core PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(abys_core PRIVATE ${ABYS_SLANG_TARGET})
target_compile_definitions(abys_core PRIVATE ABYS_HAVE_SLANG=1)

add_executable(abys src/main.cpp)

target_link_libraries(abys PRIVATE abys_core)

if(ABYS_ENABLE_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()
