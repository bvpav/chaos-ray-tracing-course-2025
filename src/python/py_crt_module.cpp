#include "core/crt_image.h"
#include "core/crt_json.h"
#include "core/crt_renderer.h"
#include "core/crt_scene.h"
#include <Python.h>

#include <boolobject.h>
#include <experimental/scope>

#include <floatobject.h>
#include <listobject.h>
#include <modsupport.h>
#include <object.h>
#include <pyerrors.h>
#include <pyport.h>
#include <pytypedefs.h>
#include <string>
#include <tupleobject.h>

constexpr int MAX_RAY_DEPTH = 5;
constexpr int DIFFUSE_REFLECTION_RAY_COUNT = 3;

constexpr float SHADOW_BIAS = 1e-2f;
constexpr float REFLECTION_BIAS = 1e-2f;
constexpr float DIFFUSE_REFLECTION_BIAS = 1e-2f;
constexpr float REFRACTION_BIAS = 1e-2f;

using std::experimental::scope_exit;

static PyObject *render_scene_from_dict([[maybe_unused]] PyObject *self, PyObject *args) {
    PyObject *dict_obj, *asset_root_unicode;
    if (!PyArg_ParseTuple(args, "O!U", &PyDict_Type, &dict_obj, &asset_root_unicode)) 
        return nullptr;

    PyObject *json_unicode;
    {
        PyObject *json_module = PyImport_ImportModule("json");
        if (!json_module)
            return nullptr;
        scope_exit json_module_guard{ [&](){ Py_DECREF(json_module); } };

        PyObject *json_dumps_func = PyObject_GetAttrString(json_module, "dumps");
        if (!json_dumps_func)
            return nullptr;
        scope_exit json_dumps_func_guard{ [&](){ Py_DECREF(json_dumps_func); } };

        PyObject *kwargs = Py_BuildValue("{sO}", "ensure_ascii", Py_True);
        if (!kwargs) 
            return nullptr;
        scope_exit kwargs_guard{ [&](){ Py_DECREF(kwargs); }};
        
        json_unicode = PyObject_Call(json_dumps_func, PyTuple_Pack(1, dict_obj), kwargs);
        if (!json_unicode)
            return nullptr;
    }

    scope_exit kwargs_guard{ [&](){ Py_DECREF(json_unicode); }};

    Py_ssize_t json_utf8_size;
    const char *json_utf8_bytes = PyUnicode_AsUTF8AndSize(json_unicode, &json_utf8_size);
    if (!json_utf8_bytes)
        return nullptr;
    std::stringstream json_ss{ std::string { json_utf8_bytes, json_utf8_bytes + json_utf8_size } };

    Py_ssize_t asset_root_utf8_size;
    const char *asset_root_utf8_bytes = PyUnicode_AsUTF8AndSize(asset_root_unicode, &asset_root_utf8_size);
    std::filesystem::path asset_root{ std::u8string {  asset_root_utf8_bytes, asset_root_utf8_bytes + asset_root_utf8_size  } };

    std::optional<crt::Scene> scene = crt::json::read_scene_from_istream(json_ss, asset_root);
    if (!scene) {
        PyErr_SetString(PyExc_ValueError, "Invalid CRT Scene dict");
        return nullptr;
    }

    crt::RendererSettings settings{
        .max_ray_depth = MAX_RAY_DEPTH,
        .diffuse_reflection_ray_count = DIFFUSE_REFLECTION_RAY_COUNT,
        .shadow_bias = SHADOW_BIAS,
        .reflection_bias = REFLECTION_BIAS,
        .diffuse_reflection_bias = DIFFUSE_REFLECTION_BIAS,
        .refraction_bias = REFRACTION_BIAS,
    };

    crt::Image image = crt::render_image(*scene, settings);

    PyObject *output_buffer = PyList_New(image.width * image.height);
    if (!output_buffer)
        return nullptr;

    for (int raster_y = 0; raster_y < image.height; ++raster_y) {
        for (int raster_x = 0; raster_x < image.width; ++raster_x) {
            const crt::Color &color = image.buffer[raster_y * image.width + raster_x];
            PyObject *color_tuple = Py_BuildValue("ffff", color.x, color.y, color.z, 1.0f);
            if (!color_tuple) {
                Py_DECREF(output_buffer);
                return nullptr;
            }
            PyList_SET_ITEM(output_buffer, (image.height - raster_y - 1) * image.width + raster_x, color_tuple);
        }
    }

    return output_buffer;
}

static PyMethodDef methods[]{
    { "render_scene_from_dict", (PyCFunction)render_scene_from_dict, METH_VARARGS },
    { nullptr, nullptr }
};

static PyModuleDef module{
    PyModuleDef_HEAD_INIT,
    "_crt",
    nullptr,
    -1,
    methods
};


PyMODINIT_FUNC PyInit__crt() {
    PyObject *module_obj = PyModule_Create(&module);
    if (!module_obj)
        return nullptr;

    if (PyModule_AddIntConstant(module_obj, "DEFAULT_SCENE_BUCKET_SIZE", crt::DEFAULT_SCENE_BUCKET_SIZE) < 0)
        goto error;

    return module_obj;
error:
    Py_DECREF(module_obj);
    return nullptr;
}