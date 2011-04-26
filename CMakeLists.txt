cmake_minimum_required(VERSION 2.6)
project(redstone C)
set(LIBREDSTONE_VERSION "0.0.0")

# custom modules
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# options
option(BUILD_SHARED_LIBS "build shared libraries" ON)

# cpack stuff
set(CPACK_SOURCE_GENERATOR "TGZ;TBZ2")
set(CPACK_SOURCE_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}-${LIBREDSTONE_VERSION}")
set(CPACK_SOURCE_IGNORE_FILES "/build/;/local/;/.git/;${CPACK_SOURCE_IGNORE_FILES}")
include(CPack)
add_custom_target(dist COMMAND ${CMAKE_MAKE_PROGRAM} package_source)

# subdirs
add_subdirectory(src)
add_subdirectory(tools)
add_subdirectory(doc)
