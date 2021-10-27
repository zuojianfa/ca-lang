add_library(ir1 STATIC dwarf_debug.cpp ir1.cpp)

target_include_directories(ir1 PUBLIC .)

install(TARGETS ir1 DESTINATION lib)
install(FILES IR_generator.h dwarf_debug.h ir1.h jit1.h DESTINATION include)

