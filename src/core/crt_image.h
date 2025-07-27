#pragma once

#include "crt_vector.h"

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

    Image(const int width, const int height, std::vector<Color> &&buffer)
        : width(width)
        , height(height)
        , buffer(std::move(buffer))
    {
    }
};

}