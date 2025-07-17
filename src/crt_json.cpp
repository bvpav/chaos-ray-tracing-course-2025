#include "crt_json.h"

#include <cassert>
#include <cstddef>
#include <new>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>
#include <rapidjson/rapidjson.h>

#include "crt_camera.h"
#include "crt_image.h"
#include "crt_image_stbi.h"
#include "crt_light.h"
#include "crt_material.h"
#include "crt_matrix.h"
#include "crt_mesh.h"
#include "crt_scene.h"
#include "crt_texture.h"
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

        auto uvs_it = v.FindMember("uvs");
        if (uvs_it == v.MemberEnd())
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

        std::optional<std::vector<Vector>> uvs = get_vector_array_from_value(uvs_it->value);
        if (!uvs)
            return std::nullopt;

        std::optional<std::vector<int>> indices = get_int_array_from_value(indices_it->value);
        if (!indices)
            return std::nullopt;

        meshes.emplace_back(std::move(*vertices), std::move(*uvs), std::move(*indices), material_index_it->value.GetInt());
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

static std::optional<MaterialType> get_material_type_from_value(const rapidjson::Value &value) {
    if (value == "diffuse")
        return MaterialType::Diffuse;
    if (value == "reflective")
        return MaterialType::Reflective;
    if (value == "refractive")
        return MaterialType::Refractive;
    if (value == "constant")
        return MaterialType::Constant;

    return std::nullopt;
}

static std::optional<TextureType> get_texture_type_from_value(const rapidjson::Value &value) {
    if (value == "albedo")
        return TextureType::Albedo;
    if (value == "edges")
        return TextureType::Edges;
    if (value == "checker")
        return TextureType::Checker;
    if (value == "bitmap")
        return TextureType::Bitmap;

    return std::nullopt;
}

static std::optional<AlbedoTexture> get_albedo_texture_from_value(const rapidjson::Value &value) {
    assert(value.IsObject());

    auto albedo_it = value.FindMember("albedo");
    if (albedo_it == value.MemberEnd())
        return std::nullopt;

    std::optional<Color> albedo_opt = get_vector_from_value(albedo_it->value);
    if (!albedo_opt)
        return std::nullopt;
    
    return AlbedoTexture { std::move(*albedo_opt) };
}

static std::optional<EdgesTexture> get_edges_texture_from_value(const rapidjson::Value &value) {
    assert(value.IsObject());

    auto edge_width_it = value.FindMember("edge_width");
    if (edge_width_it == value.MemberEnd() || !edge_width_it->value.IsNumber())
        return std::nullopt;

    auto edge_color_it = value.FindMember("edge_color");
    if (edge_color_it == value.MemberEnd())
        return std::nullopt;

    std::optional<Color> edge_color = get_vector_from_value(edge_color_it->value);
    if (!edge_color)
        return std::nullopt;
    
    auto inner_color_it = value.FindMember("inner_color");
    if (inner_color_it == value.MemberEnd())
        return std::nullopt;

    std::optional<Color> inner_color = get_vector_from_value(inner_color_it->value);
    if (!inner_color)
        return std::nullopt;
    
    return EdgesTexture {
        .edge_color = std::move(*edge_color),
        .inner_color = std::move(*inner_color),
        .edge_width = edge_width_it->value.GetFloat()
    };
}

static std::optional<CheckerTexture> get_checker_texture_from_value(const rapidjson::Value &value) {
    assert(value.IsObject());

    auto color_a_it = value.FindMember("color_A");
    if (color_a_it == value.MemberEnd())
        return std::nullopt;

    auto color_b_it = value.FindMember("color_B");
    if (color_b_it == value.MemberEnd())
        return std::nullopt;

    auto square_size_it = value.FindMember("square_size");
    if (square_size_it == value.MemberEnd() || !square_size_it->value.IsNumber())
        return std::nullopt;

    std::optional<Color> color_a = get_vector_from_value(color_a_it->value);
    if (!color_a)
        return std::nullopt;

    std::optional<Color> color_b = get_vector_from_value(color_b_it->value);
    if (!color_b)
        return std::nullopt;
    
    return CheckerTexture {
        .color_a = std::move(*color_a),
        .color_b = std::move(*color_b),
        .square_size = square_size_it->value.GetFloat(),
    };
}

static std::optional<BitmapTexture> get_bitmap_texture_from_value(const rapidjson::Value &value, const std::filesystem::path &asset_root) {
    assert(value.IsObject());

    namespace fs = std::filesystem;

    auto file_path_it = value.FindMember("file_path");
    if (file_path_it == value.MemberEnd() || !file_path_it->value.IsString())
        return std::nullopt;

    fs::path file_path{ std::u8string {  file_path_it->value.GetString(), file_path_it->value.GetString() + file_path_it->value.GetStringLength()  } };

    std::optional<Image> image = read_stb(asset_root / file_path.relative_path());
    if (!image)
        return std::nullopt;

    return BitmapTexture {
        // FIXME: Do not do this...
        .image = new Image { std::move(*image) },
    };
}

struct ParsedTextures {
    std::vector<Texture> textures;
    std::unordered_map<std::string_view, std::size_t> texture_index_map;
};

static std::optional<ParsedTextures> get_textures_from_value(const rapidjson::Value &value, const std::filesystem::path &asset_root) {
    if (!value.IsArray())
        return std::nullopt;

    ParsedTextures result;
    result.textures.reserve(value.Size());

    for (size_t i = 0; i < value.Size(); ++i) {
        const auto &v = value[i];

        if (!v.IsObject())
            return std::nullopt;

        auto name_it = v.FindMember("name");
        if (name_it == v.MemberEnd() || !name_it->value.IsString())
            return std::nullopt;

        std::string_view name{ name_it->value.GetString(), name_it->value.GetStringLength() };

        result.texture_index_map[name] = i;

        auto type_it = v.FindMember("type");
        if (type_it == v.MemberEnd())
            return std::nullopt;

        std::optional<TextureType> type = get_texture_type_from_value(type_it->value);
        if (!type)
            return std::nullopt;

        switch (*type) {
            case TextureType::Albedo: {
                std::optional<AlbedoTexture> albedo_texture = get_albedo_texture_from_value(v);
                if (!albedo_texture)
                    return std::nullopt;

                result.textures.emplace_back(
                    Texture { .type = *type, .as_albedo_tex = std::move(*albedo_texture) }
                );
                continue;
            }

            case TextureType::Edges: {
                std::optional<EdgesTexture> edges_texture = get_edges_texture_from_value(v);
                if (!edges_texture)
                    return std::nullopt;

                result.textures.emplace_back(
                    Texture { .type = *type, .as_edges_tex = std::move(*edges_texture) }
                );
                continue;
            }

            case TextureType::Checker: {
                std::optional<CheckerTexture> checker_texture = get_checker_texture_from_value(v);
                if (!checker_texture)
                    return std::nullopt;

                result.textures.emplace_back(
                    Texture { .type = *type, .as_checker_tex = std::move(*checker_texture) }
                );
                continue;
            }

            case TextureType::Bitmap: {
                std::optional<BitmapTexture> bitmap_texture = get_bitmap_texture_from_value(v, asset_root);
                if (!bitmap_texture)
                    return std::nullopt;

                result.textures.emplace_back(
                    Texture { .type = *type, .as_bitmap_tex = std::move(*bitmap_texture) }
                );
                continue;
            }
        }
        // std::unreachable() // FIXME: Use C++23 maybe?
        assert(false);
    }

    return result;
}

static std::optional<std::vector<Material>> get_materials_from_value(const rapidjson::Value &value, const std::unordered_map<std::string_view, size_t> &texture_index_map) {
    if (!value.IsArray())
        return std::nullopt;

    std::vector<Material> materials;
    materials.reserve(value.Size());

    for (const auto &v : value.GetArray()) {
        if (!v.IsObject())
            return std::nullopt;

        auto type_it = v.FindMember("type");
        if (type_it == v.MemberEnd())
            return std::nullopt;

        auto smooth_shading_it = v.FindMember("smooth_shading");
        if (smooth_shading_it == v.MemberEnd() || !smooth_shading_it->value.IsBool())
            return std::nullopt;

        std::optional<MaterialType> type = get_material_type_from_value(type_it->value);
        if (!type)
            return std::nullopt;

        float ior = 1.0f;

        int albedo_map_texture_index = -1;
        if (type != MaterialType::Refractive) {
            auto albedo_it = v.FindMember("albedo");
            if (albedo_it == v.MemberEnd() || !albedo_it->value.IsString())
                return std::nullopt;
            
            std::string_view albedo{ albedo_it->value.GetString(), albedo_it->value.GetStringLength() };

            auto albedo_map_texture_index_it = texture_index_map.find(albedo);
            if (albedo_map_texture_index_it == texture_index_map.end())
                return std::nullopt;

            albedo_map_texture_index = albedo_map_texture_index_it->second;
        } else {
            auto ior_it = v.FindMember("ior");
            // HACK: The `11-01-refractive/scene0.crtscene` scene has an unused refractive material with an albedo, instead of an IOR.
            //       I believe that to be an error in the file itself and I think IOR should be required for refractive materials.
            //       This is done to support lodading the official file, but it may be removed in the future.
            if (ior_it != v.MemberEnd()) {
                if (!ior_it->value.IsNumber())
                    return std::nullopt;
                ior = ior_it->value.GetFloat();
            }
        }

        materials.emplace_back(*type, albedo_map_texture_index, ior, smooth_shading_it->value.GetBool());
    }

    return materials;
}

std::optional<Scene> read_scene_from_istream(std::istream &is, const std::filesystem::path &asset_root) {
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

    auto textures_it = doc.FindMember("textures");
    if (textures_it == doc.MemberEnd())
        return std::nullopt;

    std::optional<ParsedTextures> parsed_textures = get_textures_from_value(textures_it->value, asset_root);
    if (!parsed_textures)
        return std::nullopt;

    std::optional<std::vector<Material>> materials = get_materials_from_value(materials_it->value, parsed_textures->texture_index_map);
    if (!materials)
        return std::nullopt;

    return Scene {
        .background_color = std::move(*bg_color),
        .camera = std::move(*camera),
        .textures = std::move(parsed_textures->textures),
        .meshes = std::move(*meshes),
        .lights = std::move(*lights),
        .materials = std::move(*materials)
    };
}

}