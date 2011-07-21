include_directories("${PROJECT_SOURCE_DIR}/src")

add_executable(mcrtool mcrtool.c)
target_link_libraries(mcrtool redstone)

add_executable(exmaple-trim exmaple-trim.c)
target_link_libraries(exmaple-trim redstone)

add_executable(nbttool nbttool.c)
target_link_libraries(nbttool redstone)

add_executable(nbtwritetest nbtwritetest.c)
target_link_libraries(nbtwritetest redstone)

add_executable(mapgen mapgen.c)
target_link_libraries(mapgen redstone)

install(TARGETS mcrtool nbttool DESTINATION bin)
