cmake_minimum_required (VERSION 2.6)
project (linuxrtm)
include_directories(
   "${PROJECT_SOURCE_DIR}")
add_executable(linuxrtm cpumon.c linuxcpu.c comm.c ../common/util.c)
target_link_libraries (linuxrtm rt pthread)
