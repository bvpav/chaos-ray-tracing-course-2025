#include "crt_json.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_matrix.h"
#include "crt_mesh.h"
#include "crt_scene.h"
#include "crt_transform.h"
#include "crt_vector.h"
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/rapidjson.h"
#include <cassert>
#include <optional>
#include <type_traits>
#include <vector>

namespace crt::json {

static std::optional<Vector> get_vector_from_value(const rapidjson::Value &value) {
    if (!value.IsArray() || value.Size() != 3)
        return std::nullopt;

    const auto &x = value[0], &y = value[1], &z = value[2];
    if (!x.IsNumber() || !y.IsNumber() || !z.IsNumber())
        return std::nullopt;

    return Vector { x.GetFloat(), y.GetFloat(), z.GetFloat() };
}

static std::optional<Matrix> get_matrix_from_value(const rapidjson::Value &value) {
    if (!value.IsArray() || value.Size() != 3 * 3)
        return std::nullopt;

    Matrix result{};

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            const auto &v = value[i * 3 + j];
            if (!v.IsNumber())
                return std::nullopt;
            result.data[i][j] = v.GetFloat();
        }
    }

    return result;
}

static std::optional<std::vector<int>> get_int_array_from_value(const rapidjson::Value &value) {
    if (!value.IsArray())
        return std::nullopt;

    std::vector<int> result;
    result.reserve(value.Size());

    for (const auto &v : value.GetArray()) {
        if (!v.IsInt())
            return std::nullopt;
        result.emplace_back(v.GetInt());
    }

    return result;
}

static std::optional<std::vector<Vector>> get_vector_array_from_value(const rapidjson::Value &value) {
    if (!value.IsArray() || value.Size() % 3 != 0)
        return std::nullopt;

    std::vector<Vector> result;
    result.reserve(value.Size() / 3);

    for (rapidjson::SizeType i = 0; i < value.Size(); i += 3) {
        const auto &x = value[i], &y = value[i + 1], &z = value[i + 2];
        if (!x.IsNumber() || !y.IsNumber() || !z.IsNumber())
            return std::nullopt;
        result.emplace_back(Vector { x.GetFloat(), y.GetFloat(), z.GetFloat() });
    }

    return result;
}

static std::optional<Transform> get_transform_from_value(const rapidjson::Value &value) {
    if (!value.IsObject())
        return std::nullopt;

    auto location_it = value.FindMember("position");
    if (location_it == value.MemberEnd())
        return std::nullopt;

    std::optional<Vector> location = get_vector_from_value(location_it->value);
    if (!location)
        return std::nullopt;

    auto rotation_matrix_it = value.FindMember("matrix");
    if (rotation_matrix_it == value.MemberEnd())
        return std::nullopt;

    std::optional<Matrix> rotation_matrix = get_matrix_from_value(rotation_matrix_it->value);
    if (!rotation_matrix)
        return std::nullopt;

    return Transform { std::move(*location), std::move(*rotation_matrix) };
}

static std::optional<Camera> get_camera_from_value(const rapidjson::Value &camera_value, const rapidjson::Value &settings_object_value) {
    assert(settings_object_value.IsObject());

    auto image_settings_it = settings_object_value.FindMember("image_settings");
    if (image_settings_it == settings_object_value.MemberEnd())
        return std::nullopt;

    auto width_it = image_settings_it->value.FindMember("width");
    if (width_it == image_settings_it->value.MemberEnd() || !width_it->value.IsInt())
        return std::nullopt;

    auto height_it = image_settings_it->value.FindMember("height");
    if (height_it == image_settings_it->value.MemberEnd() || !height_it->value.IsInt())
        return std::nullopt;

    std::optional<Transform> transform = get_transform_from_value(camera_value);
    if (!transform)
        return std::nullopt;

    return Camera { width_it->value.GetInt(), height_it->value.GetInt(), std::move(*transform) };
}

static std::optional<std::vector<Mesh>> get_meshes_from_value(const rapidjson::Value &value) {
    if (!value.IsArray())
        return std::nullopt;

    std::vector<Mesh> meshes;
    meshes.reserve(value.Size());

    for (const auto &v : value.GetArray()) {
        if (!v.IsObject())
            return std::nullopt;

        auto vertices_it = v.FindMember("vertices");
        if (vertices_it == v.MemberEnd())
            return std::nullopt;

        auto indices_it = v.FindMember("triangles");
        if (indices_it == v.MemberEnd())
            return std::nullopt;

        std::optional<std::vector<Vector>> vertices = get_vector_array_from_value(vertices_it->value);
        if (!vertices)
            return std::nullopt;

        std::optional<std::vector<int>> indices = get_int_array_from_value(indices_it->value);
        if (!indices)
            return std::nullopt;

        meshes.emplace_back(std::move(*vertices), std::move(*indices));
    }

    return meshes;
}

std::optional<Scene> get_scene_from_istream(std::istream &is) {
    rapidjson::IStreamWrapper isw{is};
    rapidjson::Document doc;
    if (doc.ParseStream(isw).HasParseError())
        return std::nullopt;

    if (!doc.IsObject())
        return std::nullopt;

    auto settings_it = doc.FindMember("settings");
    if (settings_it == doc.MemberEnd() || !settings_it->value.IsObject())
        return std::nullopt;

    auto bg_color_it = settings_it->value.FindMember("background_color");
    if (bg_color_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<Color> bg_color = get_vector_from_value(bg_color_it->value);
    if (!bg_color)
        return std::nullopt;

    auto camera_it = doc.FindMember("camera");
    if (camera_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<Camera> camera = get_camera_from_value(camera_it->value, settings_it->value);
    if (!camera)
        return std::nullopt;

    auto meshes_it = doc.FindMember("objects");
    if (meshes_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<std::vector<Mesh>> meshes = get_meshes_from_value(meshes_it->value);
    if (!meshes)
        return std::nullopt;

    return Scene { std::move(*bg_color), std::move(*meshes), std::move(*camera) };
}

}