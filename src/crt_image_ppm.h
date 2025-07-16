#pragma once

#include "crt_image.h"
#include <ostream>

namespace crt {

void write_ppm(const Image &image, std::ostream &os, int max_color_component = 255);

}