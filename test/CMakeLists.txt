add_executable(visualizer_tests main.cpp VisualizerConfigurationTests.cpp)
target_link_libraries(visualizer_tests PRIVATE visualizer doctest::doctest)
set_target_properties(visualizer_tests PROPERTIES CXX_CLANG_TIDY "")
