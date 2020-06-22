set(MDH2VIS_SRC main.cpp)

add_executable(mdh2vis ${MDH2VIS_SRC})
target_include_directories(mdh2vis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdh2vis common_options nlohmann_json::nlohmann_json)