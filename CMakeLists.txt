# CMake build file for Spark
cmake_minimum_required(VERSION 2.8.4)
project(SPARK)

# Version.
set(SPARK_VERSION 0)
set(SPARK_MAJOR_REVISION 1)
set(SPARK_MINOR_REVISION 0)

# Macros we'll need.
include(CheckIncludeFile)
include(CheckIncludeFileCXX)

# Required compiler features.
set(CMAKE_REQUIRED_FLAGS "-std=c++11")
set(CMAKE_CXX_STANDARD 11)

# C Headers.
check_include_file(dirent.h SPARK_HAVE_DIRENT_H)
check_include_file(math.h SPARK_HAVE_MATH_H)
check_include_file(stddef.h SPARK_HAVE_STDDEF_H)
check_include_file(stdint.h SPARK_HAVE_STDINT_H)
check_include_file(stdio.h SPARK_HAVE_STDIO_H)
check_include_file(unistd.h SPARK_HAVE_UNISTD_H)
check_include_file(sys/stat.h SPARK_HAVE_SYS_STAT_H)

# C++ Headers.
check_include_file_cxx(algorithm SPARK_HAVE_ALGORITHM)
check_include_file_cxx(cassert SPARK_HAVE_CASSERT)
check_include_file_cxx(csignal SPARK_HAVE_CSIGNAL)
check_include_file_cxx(cstring SPARK_HAVE_CSTRING)
check_include_file_cxx(cwctype SPARK_HAVE_CWCTYPE)
check_include_file_cxx(fstream SPARK_HAVE_FSTREAM)
check_include_file_cxx(functional SPARK_HAVE_FUNCTIONAL)
check_include_file_cxx(iostream SPARK_HAVE_IOSTREAM)
check_include_file_cxx(istream SPARK_HAVE_ISTREAM)
check_include_file_cxx(memory SPARK_HAVE_MEMORY)
check_include_file_cxx(new SPARK_HAVE_NEW)
check_include_file_cxx(ostream SPARK_HAVE_OSTREAM)
check_include_file_cxx(sstream SPARK_HAVE_SSTREAM)
check_include_file_cxx(string SPARK_HAVE_STRING)
check_include_file_cxx(unordered_map SPARK_HAVE_UNORDERED_MAP)
check_include_file_cxx(unordered_set SPARK_HAVE_UNORDERED_SET)

# Include paths.
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/cspark)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/cspark) # For generated includes
include_directories(third-party/googletest/googletest/include)
include_directories(third-party/googletest/googlemock/include)

# Extra flags for GCC (C++ only)
if (CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS
      -Wall -Wextra -Werror -Wcast-align -Wpointer-arith -Wno-sign-compare
      -Wno-deprecated -Wno-unused
  )
endif (CMAKE_COMPILER_IS_GNUCXX)

# Extra flags for clang (C++ only)
if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  add_compile_options(
    -Wall -Wextra -Werror -Wcast-align -Wpointer-arith
    -Wno-deprecated -Wno-unused-parameter
    -Wno-sign-compare # googletest won't build without this
    -fno-rtti
    -fPIC
  )
  set(CMAKE_EXE_LINKER_FLAGS -lstdc++)
endif ()

# Subdirs.
add_subdirectory(cspark)
add_subdirectory(lib)
add_subdirectory(test)
add_subdirectory(third-party)

add_test(unittest test/cspark/unit/unittest)
