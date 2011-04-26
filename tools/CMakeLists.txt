include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(mcrtool mcrtool.c)
target_link_libraries(mcrtool redstone)

add_executable(nbttool nbttool.c)
target_link_libraries(nbttool redstone)

install(TARGETS mcrtool nbttool DESTINATION bin)
