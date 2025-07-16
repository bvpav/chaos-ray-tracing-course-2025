#pragma once

#include "crt_vector.h"

#include <span>
#include <vector>

namespace crt {

using Color = Vector;

struct Image {
    int width, height;
    std::vector<Color> buffer;

    Image(const int width, const int height)
        : width(width)
        , height(height)
        , buffer(width * height)
    {}

    Image(const int width, const int height, std::span<const Color> pixels)
        : width(width)
        , height(height)
        , buffer(pixels.begin(), pixels.end())
    {
    }
};

}