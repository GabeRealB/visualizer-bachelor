set(MDH2VIS_SRC main.cpp MDHConfig.hpp MDHConfig.cpp MDHOps.hpp MDHOps.cpp processing.hpp processing.cpp)

add_executable(mdh2vis ${MDH2VIS_SRC})
target_include_directories(mdh2vis PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mdh2vis common_options visconfig nlohmann_json::nlohmann_json Threads::Threads)

add_custom_command(TARGET mdh2vis
        PRE_BUILD
        COMMAND python3 ${PROJECT_SOURCE_DIR}/mdhunfold.py ${PROJECT_SOURCE_DIR}/mdh_path.txt
        ${PROJECT_SOURCE_DIR}/src/MDHOps.cpp.in ${PROJECT_SOURCE_DIR}/src/MDHOps.cpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        DEPENDS ${PROJECT_SOURCE_DIR}/mdh_path.txt)