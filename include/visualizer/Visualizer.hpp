#pragma once

#include <filesystem>

#include <visualizer/Scene.hpp>

namespace Visualizer {

Scene initialize(std::filesystem::path data);
void terminate();

}
