import json

import bpy

from .bl_crt_json import build_scene_dict


class CRTRenderEngine(bpy.types.RenderEngine):
    bl_idname = 'CRT'
    bl_label = 'CRT'

    def render(self, depsgraph):
        from . import _crt

        # HACK: use real scene instead
        scene_dict = build_scene_dict(depsgraph)
        image_settings = scene_dict['settings']['image_settings']

        result = self.begin_result(0, 0, image_settings['width'], image_settings['height'])
        layer = result.layers[0].passes['Combined']
        layer.rect = _crt.render_scene_from_dict(scene_dict, '/')  # Make sure assets are relative to system root
        self.end_result(result)


def get_compatible_panels():
    # NOTE: Blender's UI code re-uses panel classes to create node editor versions of the panels.
    #       If we want to exclude such panels (eg. DATA_PT_light), we *MUST*
    #       include both the original name and the NODE_* version.
    #       If we forget to exclude either one, they will *BOTH* still appear :/
    #       This is because Blender's helper function creates a shallow copy and
    #       both classes reference the same COMPAT_ENGINES dict.
    #       I believe this is uninentional, I wish it were documented...
    #       See: https://projects.blender.org/blender/blender/src/commit/4bffd43adc65c5f5a122fa905713ddc981f77c61/scripts/startup/bl_ui/space_node.py#L1169
    exclude_panels = {
        'DATA_PT_light',
        'DATA_PT_preview',
        'VIEWLAYER_PT_filter',
        'VIEWLAYER_PT_layer_passes',
        'NODE_DATA_PT_light',
    }

    for panel in bpy.types.Panel.__subclasses__():
        if hasattr(panel, 'COMPAT_ENGINES') and 'BLENDER_RENDER' in panel.COMPAT_ENGINES:
            if panel.__name__ not in exclude_panels:
                yield panel


classes = (CRTRenderEngine,)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    
    for panel in get_compatible_panels():
        panel.COMPAT_ENGINES.add(CRTRenderEngine.bl_idname)


def unregister():
    for panel in get_compatible_panels():
        # FIXME: For some reason, register() succeeds, but some Blender native panels at this
        #        point still don't have CRT in COMPAT_ENGINES. Maybe they are
        #        loaded after CRT? Could be a bug in Blender?
        # TODO: Investigate this
        if CRTRenderEngine.bl_idname not in panel.COMPAT_ENGINES:
            continue
        panel.COMPAT_ENGINES.remove(CRTRenderEngine.bl_idname)

    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
