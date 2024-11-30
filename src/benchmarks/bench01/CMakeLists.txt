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
)
target_link_libraries(bench01
        PRIVATE
        utils
)