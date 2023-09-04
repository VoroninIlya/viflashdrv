cmake_minimum_required(VERSION 3.22)

project(tst_viflashdrv)

#include(CodeCoverage)
include(CTest)

if(PROJECT_IS_TOP_LEVEL)
# Set as executable
add_subdirectory(${UNITY_ROOT_PATH} ${${EXECUTABLE}_BINARY_DIR}/modules/unity)
endif()

# Add core subdir
add_subdirectory(core)

add_executable(tst_viflashdrv)
enable_testing()

# Add key executable block
target_sources(tst_viflashdrv PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/tests/main/main.c
    ${CMAKE_CURRENT_LIST_DIR}/tests/tst_viflashdrv.c
)

# Add key include paths
target_include_directories(tst_viflashdrv PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/core/str/inc
    ${UNITY_ROOT_PATH}/src
    ${UNITY_ROOT_PATH}/extras/fixture/src
    ${UNITY_ROOT_PATH}/extras/memory/src
)

# Compilation definition information
target_compile_definitions(tst_viflashdrv PUBLIC
    WIN32
    _DEBUG
    CONSOLE
)

# Compiler options
target_compile_options(tst_viflashdrv PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(
  tst_viflashdrv viflashdrv unity)

add_test(NAME VIFLASH_SomeFunc COMMAND tst_viflashdrv "--gtest_filter=VIFLASH_SomeFunc.*")
