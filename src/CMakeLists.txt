find_package(ZLIB REQUIRED)

add_library(redstone memory.c endian.c error.c compression.c list.c region.c nbt.c tag.c)
target_link_libraries(redstone ${ZLIB_LIBRARIES})

install(TARGETS redstone DESTINATION lib)
install(DIRECTORY ./ DESTINATION include/redstone FILES_MATCHING PATTERN "*.h")
