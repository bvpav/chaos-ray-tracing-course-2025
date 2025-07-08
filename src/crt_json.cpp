#include "crt_json.h"

#include <cassert>
#include <optional>
#include <vector>

#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/rapidjson.h"

#include "crt_light.h"
#include "crt_material.h"
#include "crt_camera.h"
#include "crt_image.h"
#include "crt_matrix.h"
#include "crt_mesh.h"
#include "crt_scene.h"
#include "crt_transform.h"
#include "crt_vector.h"

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

        auto material_index_it = v.FindMember("material_index");
        if (material_index_it == v.MemberEnd() || !material_index_it->value.IsInt())
            return std::nullopt;

        std::optional<std::vector<Vector>> vertices = get_vector_array_from_value(vertices_it->value);
        if (!vertices)
            return std::nullopt;

        std::optional<std::vector<int>> indices = get_int_array_from_value(indices_it->value);
        if (!indices)
            return std::nullopt;

        meshes.emplace_back(std::move(*vertices), std::move(*indices), material_index_it->value.GetInt());
    }

    return meshes;
}

static std::optional<std::vector<Light>> get_lights_from_value(const rapidjson::Value &value) {
    if (!value.IsArray())
        return std::nullopt;

    std::vector<Light> lights;
    lights.reserve(value.Size());

    for (const auto &v : value.GetArray()) {
        if (!v.IsObject())
            return std::nullopt;

        auto intensity_it = v.FindMember("intensity");
        if (intensity_it == v.MemberEnd() || !intensity_it->value.IsNumber())
            return std::nullopt;

        auto position_it = v.FindMember("position");
        if (position_it == v.MemberEnd())
            return std::nullopt;

        std::optional<Vector> position = get_vector_from_value(position_it->value);
        if (!position)
            return std::nullopt;

        lights.emplace_back(intensity_it->value.GetFloat(), std::move(*position));
    }

    return lights;
}

static std::optional<MaterialType> get_material_type_from_value(const rapidjson::Value &string_value) {
    assert(string_value.IsString());

    if (string_value == "diffuse")
        return MaterialType::Diffuse;
    if (string_value == "reflective")
        return MaterialType::Reflective;

    return std::nullopt;
}

static std::optional<std::vector<Material>> get_materials_from_value(const rapidjson::Value &value) {
    if (!value.IsArray())
        return std::nullopt;

    std::vector<Material> materials;
    materials.reserve(value.Size());

    for (const auto &v : value.GetArray()) {
        if (!v.IsObject())
            return std::nullopt;

        auto type_it = v.FindMember("type");
        if (type_it == v.MemberEnd() || !type_it->value.IsString())
            return std::nullopt;

        auto albedo_it = v.FindMember("albedo");
        if (albedo_it == v.MemberEnd())
            return std::nullopt;

        auto smooth_shading_it = v.FindMember("smooth_shading");
        if (smooth_shading_it == v.MemberEnd() || !smooth_shading_it->value.IsBool())
            return std::nullopt;

        std::optional<Color> albedo = get_vector_from_value(albedo_it->value);
        if (!albedo)
            return std::nullopt;

        std::optional<MaterialType> type = get_material_type_from_value(type_it->value);
        if (type == std::nullopt)
            return std::nullopt;

        materials.emplace_back(*type, std::move(*albedo), smooth_shading_it->value.GetBool());
    }

    return materials;
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

    auto lights_it = doc.FindMember("lights");
    if (lights_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<std::vector<Light>> lights = get_lights_from_value(lights_it->value);
    if (!lights)
        return std::nullopt;

    auto materials_it = doc.FindMember("materials");
    if (materials_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<std::vector<Material>> materials = get_materials_from_value(materials_it->value);
    if (!materials)
        return std::nullopt;

    return Scene { std::move(*bg_color), std::move(*camera), std::move(*meshes), std::move(*lights), std::move(*materials) };
}

}