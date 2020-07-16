#pragma once

#include <visualizer/World.hpp>

namespace Visualizer {

class System : public GenericManager {
public:
    System(const System& other) = delete;
    System(System&& other) = default;
    virtual ~System() = default;

    System& operator=(const System& other) = delete;
    System& operator=(System&& other) = default;

    virtual void run(void* data) = 0;
    virtual void initialize() = 0;
    virtual void terminate() = 0;

protected:
    System() = default;
};

}
