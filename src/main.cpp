#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <array>

#include "crt_triangle.h"

int main(int argc, char *argv[]) {
    std::array<crt::Triangle, 3> triangles{
        crt::Triangle{{
            {-1.75f, -1.75f,  -3.0f},
            { 1.75f, -1.75f,  -3.0f},
            { 0.0f,   1.75f,  -3.0f},
        }},
        crt::Triangle{{
            { 0.0f,   0.0f,   -1.0f},
            { 1.0f,   0.0f,    1.0f},
            {-1.0f,   0.0f,    1.0f},
        }},
        crt::Triangle{{
            { 0.56f,  1.11f,   1.23f},
            { 0.44f, -2.368f, -0.54f},
            {-1.56f,  0.15f,  -1.92f},
        }}
    };
    
    for (const auto& triangle : triangles) {
        crt::Vector normal = triangle.normal();
        std::cout << "Normal = (" << normal.x << ", " << normal.y << ", " << normal.z << ")\n";
        std::cout << "Area = " << triangle.area() << '\n';
    }
    
    return 0;
}