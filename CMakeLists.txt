# CMake build file for Spark
cmake_minimum_required(VERSION 2.8.4)
project(SPARK)

# Version
set(SPARK_VERSION 0)
set(SPARK_MAJOR_REVISION 1)
set(SPARK_MINOR_REVISION 0)

# Macros we'll need
include(CheckIncludeFile)
include(CheckIncludeFileCXX)

# Headers.
check_include_file(stdint.h SPARK_HAVE_STDINT_H)
check_include_file_cxx(unordered_map SPARK_HAVE_UNORDERED_MAP)
check_include_file_cxx(unordered_set SPARK_HAVE_UNORDERED_SET)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cspark)

# Subdirs.
add_subdirectory(cspark)
add_subdirectory(test)
