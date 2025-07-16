#pragma once

#include "crt_vector.h"

#include <vector>

namespace crt {

using Color = Vector;

struct Image {
    int width, height;
    std::vector<Color> pixels;

    Image(const int width, const int height)
        : width(width)
        , height(height)
        , pixels(width * height)
    {}
};

}