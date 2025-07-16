#include "crt_debug.h"
#include <cassert>
#include <fstream>
#include <ios>
#include <iostream>
#include <mutex>

namespace crt {

void DebugLog::log_ray(const Ray ray, const float length) {
    if ((m_raster_x == 827 && m_raster_y == 410))
        m_log.emplace_back(ray, length, m_raster_x, m_raster_y);
}

void DebugLog::set_raster_coords(int x, int y) {
    m_raster_x = x, m_raster_y = y;
}

void DebugLog::flush() const {
    // HACK: I think writing to a file is faster than stdout
    // FIXME: We are never closing this file, this is probably fine...
    static std::ofstream of("file.py", std::ios::out | std::ios::binary);
    assert(of.is_open());

    static std::mutex mutex;
    std::scoped_lock<std::mutex> l(mutex);

    for (const auto &l : m_log) {
        of << "bpy.ops.crt.debug_ray_add("
           << "origin=(" << l.ray.origin.x << ", " << l.ray.origin.y << ", " << l.ray.origin.z << "), "
           << "direction=(" << l.ray.direction.x << ", " << l.ray.direction.y << ", " << l.ray.direction.z << "), "
           << "length=" << l.length << ", "
           << "depth=" << l.ray.depth << ", "
           << "raster_coords=(" << l.raster_x << ", " << l.raster_y  << "), "
           << "axis_forward='-Z', axis_up='Y'"
           << ")\n";
    }
}

}