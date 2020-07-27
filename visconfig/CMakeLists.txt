cmake_minimum_required(VERSION 3.13)

project(visconfig VERSION 1.0
        DESCRIPTION ""
        LANGUAGES CXX)

set(VISCONFIG_INC include/visconfig/Config.hpp include/visconfig/Assets.hpp include/visconfig/Components.hpp)

set(VISCONFIG_SRC src/Assets.cpp src/Components.cpp src/Config.cpp)

add_library(visconfig STATIC ${VISCONFIG_INC} ${VISCONFIG_SRC})
target_include_directories(visconfig PUBLIC include)
target_link_libraries(visconfig PUBLIC common_options nlohmann_json::nlohmann_json)