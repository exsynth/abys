add_executable(abys_tests
  frontend/lowering_test.cpp
  third_party/catch2/catch_amalgamated.cpp
  transform/blast/blast_test.cpp
  transform/decompose_dynamic_access/decompose_dynamic_access_test.cpp
  transform/fast/fast_test.cpp
  transform/flatten_subroutine/flatten_subroutine_test.cpp
  transform/infer_memory/infer_memory_test.cpp
  version_test.cpp
)

target_include_directories(abys_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} third_party/catch2)
target_link_libraries(abys_tests PRIVATE abys_lib)
target_compile_definitions(
  abys_tests PRIVATE
    ABYS_BLAST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/transform/blast/fixtures"
    ABYS_FRONTEND_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/frontend/fixtures"
    ABYS_DECOMPOSE_DYNAMIC_ACCESS_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/transform/decompose_dynamic_access/fixtures"
    ABYS_FAST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/transform/fast/fixtures"
    ABYS_FLATTEN_SUBROUTINE_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/transform/flatten_subroutine/fixtures"
    ABYS_INFER_MEMORY_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/transform/infer_memory/fixtures"
)

add_test(NAME abys_tests COMMAND abys_tests)
