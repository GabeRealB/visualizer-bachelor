#pragma once

#include <filesystem>

namespace Visualizer {

void quit();
bool shouldQuit();

void attach(bool attached);
void freeze(bool frozen);
bool isDetached();
bool is_frozen();

void getRelativeMousePosition(double& xPos, double& yPos);

int run(const std::filesystem::path& configurationPath);

}
