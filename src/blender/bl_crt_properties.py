import bpy

from . import _crt


class CRTRenderSettings(bpy.types.PropertyGroup):
    bucket_size: bpy.props.IntProperty(
        name='Bucket Size',
        default=_crt.DEFAULT_SCENE_BUCKET_SIZE,
        description='',
        min=8, max=8192,
    )
    gi_on: bpy.props.BoolProperty(
        name='GI',
        default=True,
        description='Use Global Illumination',
    )
    reflections_on: bpy.props.BoolProperty(
        name='Reflections',
        default=True,
        description=''
    )
    refractions_on: bpy.props.BoolProperty(
        name='Refractions',
        default=True,
        description=''
    )

    @classmethod
    def register(cls):
        bpy.types.Scene.crt = bpy.props.PointerProperty(
            name='CRT Render Settings',
            description='',
            type=cls,
        )

    @classmethod
    def unregister(cls):
        del bpy.types.Scene.crt


class CRTMaterialProperties(bpy.types.PropertyGroup):
    smooth_shading: bpy.props.BoolProperty(name='Smooth Shading', default=False)

    @classmethod
    def register(cls):
        bpy.types.Material.crt = bpy.props.PointerProperty(
            name='CRT Material Properties',
            description='',
            type=cls,
        )
    
    @classmethod
    def unregister(cls):
        del bpy.type.Material.crt
    

classes = (
    CRTRenderSettings,
    CRTMaterialProperties,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
