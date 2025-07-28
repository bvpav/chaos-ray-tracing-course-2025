#include <Python.h>

#include <experimental/scope>
#include <modsupport.h>
#include <moduleobject.h>
#include <object.h>
#include <structseq.h>

#include "core/crt_image.h"
#include "core/crt_json.h"
#include "core/crt_renderer.h"
#include "core/crt_scene.h"

using std::experimental::scope_exit;

static PyStructSequence_Field fields[]{
    { "max_ray_depth",                "Maximum recursion depth for rays" },
    { "diffuse_reflection_ray_count", "Number of rays for diffuse reflections" },
    { "shadow_bias",                  "Epsilon used for shadow acne avoidance" },
    { "reflection_bias",              "Epsilon used for reflection acne avoidance" },
    { "diffuse_reflection_bias",      "Epsilon used for diffuse reflection acne avoidance" },
    { "refraction_bias",              "Epsilon used for refraction acne avoidance" },
    { nullptr }
};

static PyStructSequence_Desc renderer_settings_desc = {
    .name = "_crt.RendererSettings",
    .doc = "Renderer settings used by crt_core",
    .fields = fields,
    .n_in_sequence = 6
};

static PyTypeObject *RendererSettingsType = nullptr;

bool get_renderer_settings(PyObject *obj, crt::RendererSettings &result) {
    if (!PyObject_TypeCheck(obj, RendererSettingsType)) {
        PyErr_SetString(PyExc_TypeError, "Expected a RendererSettings instance");
        return false;
    }

    result.max_ray_depth = PyLong_AsUnsignedLong(PyStructSequence_GET_ITEM(obj, 0));
    result.diffuse_reflection_ray_count = PyLong_AsUnsignedLong(PyStructSequence_GET_ITEM(obj, 1));
    result.shadow_bias = static_cast<float>(PyFloat_AsDouble(PyStructSequence_GET_ITEM(obj, 2)));
    result.reflection_bias = static_cast<float>(PyFloat_AsDouble(PyStructSequence_GET_ITEM(obj, 3)));
    result.diffuse_reflection_bias = static_cast<float>(PyFloat_AsDouble(PyStructSequence_GET_ITEM(obj, 4)));
    result.refraction_bias = static_cast<float>(PyFloat_AsDouble(PyStructSequence_GET_ITEM(obj, 5)));

    return !PyErr_Occurred();
}

static PyObject *render_scene_from_dict([[maybe_unused]] PyObject *self, PyObject *args) {
    PyObject *dict_obj, *asset_root_unicode, *renderer_settings_obj;
    if (!PyArg_ParseTuple(args, "O!UO", &PyDict_Type, &dict_obj, &asset_root_unicode, &renderer_settings_obj)) 
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

    scope_exit json_unicode_guard{ [&](){ Py_DECREF(json_unicode); }};

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

    crt::RendererSettings renderer_settings;
    if (!get_renderer_settings(renderer_settings_obj, renderer_settings))
        return nullptr;

    crt::Image image = crt::render_image(*scene, renderer_settings);

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

    scope_exit module_guard{ [&](){ Py_DECREF(module_obj); } };

    if (PyModule_AddIntConstant(module_obj, "DEFAULT_SCENE_BUCKET_SIZE", crt::DEFAULT_SCENE_BUCKET_SIZE) < 0)
        return nullptr;

    if (PyModule_AddIntConstant(module_obj, "DEFAULT_MAX_RAY_DEPTH", crt::DEFAULT_MAX_RAY_DEPTH) < 0)
        return nullptr;

    if (PyModule_AddIntConstant(module_obj, "DEFAULT_DIFFUSE_REFLECTION_RAY_COUNT", crt::DEFAULT_DIFFUSE_REFLECTION_RAY_COUNT) < 0)
        return nullptr;
    if (PyModule_AddObject(module_obj, "DEFAULT_SHADOW_BIAS", PyFloat_FromDouble(crt::DEFAULT_SHADOW_BIAS)) < 0)
        return nullptr;
    if (PyModule_AddObject(module_obj, "DEFAULT_REFLECTION_BIAS", PyFloat_FromDouble(crt::DEFAULT_REFLECTION_BIAS)) < 0)
        return nullptr;
    if (PyModule_AddObject(module_obj, "DEFAULT_DIFFUSE_REFLECTION_BIAS", PyFloat_FromDouble(crt::DEFAULT_DIFFUSE_REFLECTION_BIAS)) < 0)
        return nullptr;
    if (PyModule_AddObject(module_obj, "DEFAULT_REFRACTION_BIAS", PyFloat_FromDouble(crt::DEFAULT_REFRACTION_BIAS)) < 0)
        return nullptr;

    RendererSettingsType = PyStructSequence_NewType(&renderer_settings_desc);
    if (!RendererSettingsType)
        return nullptr;

    Py_INCREF(RendererSettingsType);
    if (PyModule_AddObject(module_obj, "RendererSettings", (PyObject *)RendererSettingsType) < 0)
        return nullptr;

    module_guard.release();
    return module_obj;
}