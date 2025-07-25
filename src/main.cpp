#include <chrono>
#include <fstream>
#include <iostream>

#include "crt_image.h"
#include "crt_image_ppm.h"
#include "crt_json.h"
#include "crt_renderer.h"
#include "crt_scene.h"

// TODO: move to configuration

static constexpr int MAX_COLOR_COMPONENT = 0xFF;

constexpr int MAX_RAY_DEPTH = 5;

constexpr float SHADOW_BIAS = 1e-2f;
constexpr float REFLECTION_BIAS = 1e-2f;
constexpr float REFRACTION_BIAS = 1e-2f;

int main(int argc, char *argv[]) {
    using namespace std::chrono;

    std::filesystem::path input_file_path = argc > 1 ? argv[1] : "../scenes/14-01-acceleration-tree/scene1.crtscene";

    std::ifstream input_file{ input_file_path, std::ios::in | std::ios::binary };
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_file_path << '\n';
        return 1;
    }

    std::optional<crt::Scene> scene = crt::json::read_scene_from_istream(input_file, input_file_path.parent_path());
    if (!scene) {
        std::cerr << "Error: Could not parse JSON file: " << input_file_path << '\n';
        return 1;
    }

    auto output_file_path = argc > 2 ? argv[2] : "output.ppm";
    std::ofstream output_file{ output_file_path, std::ios::out | std::ios::binary };
    if (!output_file.is_open()) {
        std::cerr << "Error: Could not open output file: " << output_file_path << '\n';
        return 1;
    }

    crt::RendererSettings settings{
        .max_ray_depth = MAX_RAY_DEPTH,
        .shadow_bias = SHADOW_BIAS,
        .reflection_bias = REFLECTION_BIAS,
        .refraction_bias = REFRACTION_BIAS,
    };

    high_resolution_clock::time_point start = high_resolution_clock::now();
    crt::Image image = crt::render_image(*scene, settings);
    high_resolution_clock::time_point stop = high_resolution_clock::now();

    microseconds duration = duration_cast<microseconds>(stop - start);
    const long double seconds = duration.count() / 1'000'000.0l;
    std::cout << "Execution time: " << seconds << " seconds.\n";

    crt::write_ppm(image, output_file);

    return 0;
}
