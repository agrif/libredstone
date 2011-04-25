include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(mcrtool mcrtool.c)
target_link_libraries(mcrtool redstone)

install(TARGETS mcrtool DESTINATION bin)
