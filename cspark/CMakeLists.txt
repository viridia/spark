# Build rules for cspark libraries and tools.

# Generate the config.h file
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/spark/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/spark/config.h)

file(GLOB headers spark/*/*.h)

# Compiler Library
file(GLOB compiler_sources
  spark/ast/*.cpp
  spark/collections/*.cpp
  spark/compiler/*.cpp
  spark/error/*.cpp
  spark/parse/*.cpp
  spark/scope/*.cpp
  spark/sema/*.cpp
  spark/sema/*/*.cpp
  spark/semgraph/*.cpp
  spark/source/*.cpp
  spark/support/*.cpp
)
add_library(compiler STATIC ${compiler_sources} ${headers})
set_property(TARGET compiler PROPERTY CXX_STANDARD 11)

add_executable(cspark "cspark.cpp")
target_link_libraries(cspark compiler)
set_property(TARGET cspark PROPERTY CXX_STANDARD 11)
