add_library(ir1 STATIC dwarf_debug.cpp ir1.cpp)

target_compile_options(ir1 PRIVATE ${llvm_cxxflags})

target_include_directories(ir1 PRIVATE .)

install(TARGETS ir1 DESTINATION lib)
install(FILES IR_generator.h dwarf_debug.h ir1.h jit1.h DESTINATION include)

if(LINUX)
  message("running in linux")
endif()


