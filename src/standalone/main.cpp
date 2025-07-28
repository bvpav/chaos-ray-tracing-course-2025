#include <chrono>
#include <fstream>
#include <iostream>

#include "core/crt_image.h"
#include "core/crt_image_ppm.h"
#include "core/crt_json.h"
#include "core/crt_renderer.h"
#include "core/crt_scene.h"

int main(int argc, char *argv[]) {
    using namespace std::chrono;

    std::filesystem::path input_file_path = argc > 1 ? argv[1] : "../scenes/15-01-conclusion/scene2.crtscene";

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

    crt::RendererSettings settings;

    high_resolution_clock::time_point start = high_resolution_clock::now();
    crt::Image image = crt::render_image(*scene, settings);
    high_resolution_clock::time_point stop = high_resolution_clock::now();

    microseconds duration = duration_cast<microseconds>(stop - start);
    const long double seconds = duration.count() / 1'000'000.0l;
    std::cout << "Execution time: " << seconds << " seconds.\n";

    crt::write_ppm(image, output_file);

    return 0;
}
