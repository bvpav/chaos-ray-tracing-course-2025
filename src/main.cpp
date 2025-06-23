#include <iostream>
#include <fstream>
#include <cstdint>
#include <algorithm>

#include "crt_triangle.h"

int main(int argc, char *argv[]) {
    {
        crt::Vector a = { 3.50f, 0.0f, 0.0f };
        crt::Vector b = { 1.75f, 3.5f, 0.0f };
        crt::Vector cross = a.cross(b);
        std::cout << "AxB = (" << cross.x << ", " << cross.y << ", " << cross.z << ")\n";
    }
    {
        crt::Vector a = { 3.0f, -3.0f, 1.0f };
        crt::Vector b = { 4.0f,  9.0f, 3.0f };
        crt::Vector cross = a.cross(b);
        std::cout << "AxB = (" << cross.x << ", " << cross.y << ", " << cross.z << ")\n";
    }
    {
        crt::Vector a = { 3.0f, -3.0f, 1.0f };
        crt::Vector b = { 4.0f,  9.0f, 3.0f };
        std::cout << "Area = " << a.cross(b).length() << '\n';
    }
    {
        crt::Vector a = {   3.0f, -3.0f,  1.0f };
        crt::Vector b = { -12.0f, 12.0f, -4.0f };
        std::cout << "Area = " << a.cross(b).length() << '\n';
    }
    return 0;
}