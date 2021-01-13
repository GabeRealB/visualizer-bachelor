set(CONF_SRC Config.hpp config.cpp main.cpp)

add_executable(conf2 ${CONF_SRC})
target_include_directories(conf2 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(conf2 common_options visconfig nlohmann_json::nlohmann_json Threads::Threads)

file(READ ${PROJECT_SOURCE_DIR}/config_path.txt CONFIG_PATH)

add_custom_command(TARGET conf2
        PRE_BUILD
        COMMAND python3 ${PROJECT_SOURCE_DIR}/code_generator.py generate ${CONFIG_PATH}
        ${PROJECT_SOURCE_DIR}/src/config_template.in ${PROJECT_SOURCE_DIR}/src/config.cpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS ${PROJECT_SOURCE_DIR}/config_path.txt)