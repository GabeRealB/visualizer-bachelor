add_executable(visualizer_tests main.cpp)
target_link_libraries(visualizer_tests PRIVATE visualizer doctest::doctest)
set_target_properties(visualizer_tests PROPERTIES CXX_CLANG_TIDY "")

add_custom_command(
        TARGET visualizer_tests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/config.json
        $<TARGET_FILE_DIR:visualizer_tests>)
