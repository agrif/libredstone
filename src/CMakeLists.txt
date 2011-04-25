add_library(redstone memory.c region.c)

install(TARGETS redstone DESTINATION lib)
install(DIRECTORY ./ DESTINATION include/redstone FILES_MATCHING PATTERN "*.h")
