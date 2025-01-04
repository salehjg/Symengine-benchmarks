add_library(bench05 "")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
link_libraries(${JSONCPP_LIBRARIES})
find_package(Boost REQUIRED COMPONENTS filesystem)

# target_compile_options(utils PRIVATE "")
target_sources(bench05
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/bench05.cpp
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/bench05.h
        ${CMAKE_CURRENT_LIST_DIR}/CFileWriterBase.h
        ${CMAKE_CURRENT_LIST_DIR}/CFileWriter.h
        ${CMAKE_CURRENT_LIST_DIR}/CClonedExprReconstruction.h
)
target_include_directories(bench05
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/..
        ${CMAKE_CURRENT_LIST_DIR}/../../../symengine
        ${CMAKE_BINARY_DIR}/symengine
        ${CMAKE_BINARY_DIR}/symengine/symengine/utilities/teuchos/
)
target_link_libraries(bench05
        PRIVATE
        utils
        ${JSONCPP_LIBRARIES}
        Boost::filesystem
        PUBLIC
        symengine
)

add_executable(bench05_main bench_main.cpp)
target_link_libraries(bench05_main PRIVATE utils bench05)

# copy the bash script to the build directory
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/plot.sh DESTINATION ${CMAKE_CURRENT_BINARY_DIR})