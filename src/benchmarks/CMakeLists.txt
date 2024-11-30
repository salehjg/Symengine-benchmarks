add_subdirectory(bench01)

# Copy the python code to the build directory of this folder
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/plot_mem_usage.py DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
