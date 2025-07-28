import json

import bpy

from .bl_crt_json import build_scene_dict


class CRTRenderEngine(bpy.types.RenderEngine):
    bl_idname = 'CRT'
    bl_label = 'CRT'
    bl_use_preview = True

    def render(self, depsgraph):
        from . import _crt

        # HACK: use real scene instead
        scene_dict = build_scene_dict(depsgraph)
        image_settings = scene_dict['settings']['image_settings']
        
        result = self.begin_result(0, 0, image_settings['width'], image_settings['height'])
        layer = result.layers[0].passes["Combined"]
        output_buffer = _crt.render_scene_from_dict(scene_dict, '/')  # Make sure assets are relative to system root
        layer.rect = output_buffer
        self.end_result(result)


classes = (CRTRenderEngine,)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
