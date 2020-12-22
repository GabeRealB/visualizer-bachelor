#pragma once

namespace MDH2Vis {

template <typename T> float interpolate_linear(float start_value, float end_value, T min, T max, T current)
{
    if (current <= min) {
        return start_value;
    } else if (current >= max) {
        return end_value;
    } else {
        auto length{ max - min };
        auto min_distance{ current - min };
        auto normalized_min_distance{ static_cast<float>(min_distance) / static_cast<float>(length) };

        auto start_influence{ (1 - normalized_min_distance) * start_value };
        auto end_influence{ normalized_min_distance * end_value };
        return start_influence + end_influence;
    }
}

}