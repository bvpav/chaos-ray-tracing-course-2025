import json

import bpy
from bpy_extras.io_utils import ExportHelper, ImportHelper, axis_conversion

from .bl_crt_json import build_scene_dict, load_scene_dict

class EXPORT_SCENE_OT_crtscene(bpy.types.Operator, ExportHelper):
    """Export scene to CRT scene JSON format."""

    bl_idname = 'export_scene.crt_scene'
    bl_label = 'Export CRT Scene'

    filename_ext = '.crtscene'

    filter_glob: bpy.props.StringProperty(
        default='*.crtscene',
        options={'HIDDEN'},
    )

    def execute(self, context):
        depsgraph = context.evaluated_depsgraph_get()
        scene_dict = build_scene_dict(depsgraph)
        with open(self.filepath, 'w') as f:
            json.dump(scene_dict, f, indent=2)
        return {'FINISHED'}


class ImportCRTScene(bpy.types.Operator, ImportHelper):
    """Import scene from CRT scene JSON format."""

    bl_idname = 'import_scene.crt_scene'
    bl_label = 'Import CRT Scene'

    filename_ext = '.crtscene'

    filter_glob: bpy.props.StringProperty(
        default='*.crtscene',
        options={'HIDDEN'},
    )

    def execute(self, context):
        with open(self.filepath, 'r') as f:
            scene_dict = json.load(f)
        load_scene_dict(scene_dict)
        return {'FINISHED'}


def menu_func_export(self, context):
    self.layout.operator(EXPORT_SCENE_OT_crtscene.bl_idname, text='CRT Scene (.crtscene)')


def menu_func_import(self, context):
    self.layout.operator(ImportCRTScene.bl_idname, text='CRT Scene (.crtscene)')


classes = (
    EXPORT_SCENE_OT_crtscene,
    ImportCRTScene,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)
    bpy.types.TOPBAR_MT_file_export.append(menu_func_export)
    bpy.types.TOPBAR_MT_file_import.append(menu_func_import)


def unregister():
    for cls in classes:
        bpy.utils.unregister_class(cls)
    bpy.types.TOPBAR_MT_file_export.remove(menu_func_export)
    bpy.types.TOPBAR_MT_file_import.remove(menu_func_import)
