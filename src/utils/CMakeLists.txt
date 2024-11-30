add_library(utils "")
# target_compile_options(utils PRIVATE "")
target_sources(utils
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/timers.cpp
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/timers.h
        ${CMAKE_CURRENT_LIST_DIR}/mem_usage_tracker.h
)
target_include_directories(utils
        PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
        PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/..
)
#target_link_libraries(utils
#        PRIVATE
#        z
#)