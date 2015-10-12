# Build file for Spark unit tests

# Unit test.
file(GLOB unit_sources *.cpp)
add_executable(unittest ${unit_sources})
target_link_libraries(unittest compiler gtest gmock pthread)
set_property(TARGET unittest PROPERTY CXX_STANDARD 11)
