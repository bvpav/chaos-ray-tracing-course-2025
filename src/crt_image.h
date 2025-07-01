#pragma once

#include "crt_vector.h"

#include <vector>
#include <ostream>

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

    class PPM {
    public:
        explicit PPM(const Image &image, int max_color_component)
            : m_image(image)
            , m_max_color_component(max_color_component) {}
        friend std::ostream &operator<<(std::ostream &os, const PPM &ppm);
    private:
        const Image &m_image;
        int m_max_color_component;
    };

    PPM to_ppm(int max_color_component) const { return PPM{ *this, max_color_component}; }
};

}