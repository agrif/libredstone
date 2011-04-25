add_library(redstone memory.c endian.c list.c region.c)

install(TARGETS redstone DESTINATION lib)
install(DIRECTORY ./ DESTINATION include/redstone FILES_MATCHING PATTERN "*.h")
