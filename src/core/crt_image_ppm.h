#pragma once

#include <ostream>

#include "crt_image.h"

namespace crt {

void write_ppm(const Image &image, std::ostream &os, int max_color_component = 255);

}