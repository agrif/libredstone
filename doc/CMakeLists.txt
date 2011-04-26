set(DOXYGEN_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(DOXYGEN_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")

configure_file(Doxyfile.in ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "html;latex")
add_custom_target("doc" doxygen ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
# INSTALL(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/html/" DESTINATION "/usr/share/doc/libredstone-${LIBREDSTONE_VERSION}")