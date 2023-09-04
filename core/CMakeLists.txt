cmake_minimum_required(VERSION 3.22)

project(viflashdrv)

# Debug message
message("Entering ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")

# Register core library
add_library(viflashdrv INTERFACE)
target_sources(viflashdrv PUBLIC ${CMAKE_CURRENT_LIST_DIR}/src/viflashdrv.c)
target_include_directories(viflashdrv INTERFACE ${CMAKE_CURRENT_LIST_DIR}/src/inc)

# Debug message
message("Exiting ${CMAKE_CURRENT_LIST_DIR}/CMakeLists.txt")