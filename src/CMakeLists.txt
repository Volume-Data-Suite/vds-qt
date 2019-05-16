add_executable(VDS main.cpp)

target_link_libraries(VDS PRIVATE vdtk_lib)


# set C++ language standard to c++17
target_compile_features(VDS PRIVATE cxx_std_17)
