#pragma once

#include "crt_ray.h"
#include <mutex>
#include <ostream>
#include <vector>

namespace crt {

class DebugLog {
public:
    void log_ray(const Ray ray, const float length = 1.0f);
    void set_raster_coords(int x, int y);
    void flush() const;

private:
    struct LogEntry {
        Ray ray;
        float length;
        int raster_x, raster_y;
    };

    std::vector<LogEntry> m_log;
    int m_raster_x{0}, m_raster_y{0};
};

}
