add_subdirectory(benchmarks)
add_subdirectory(utils)

# add main.cpp
add_executable(main main.cpp)
target_link_libraries(main PRIVATE utils bench01)