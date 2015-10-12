# Spark libraries

file(GLOB core_sources spark/**/*.sp)

add_custom_command(
  OUTPUT core.splib
  COMMAND ${CMAKE_BINARY_DIR}/cspark/cspark ARGS ${CMAKE_CURRENT_SOURCE_DIR}/spark
  DEPENDS compiler ${core_sources}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(corelib DEPENDS core.splib)
