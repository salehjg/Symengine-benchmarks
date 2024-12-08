add_library(bench01 "")
# target_compile_options(utils PRIVATE "")
target_sources(bench01
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/bench01.cpp
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/bench01.h
)
target_include_directories(bench01
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${CMAKE_CURRENT_LIST_DIR}/../../../symengine
        ${CMAKE_BINARY_DIR}/symengine
        ${CMAKE_BINARY_DIR}/symengine/symengine/utilities/teuchos/
)
target_link_libraries(bench01
        PUBLIC
        symengine
        utils
)

add_executable(bench01_main bench_main.cpp)
target_link_libraries(bench01_main PRIVATE utils bench01)

# copy the bash script to the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/plot.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})