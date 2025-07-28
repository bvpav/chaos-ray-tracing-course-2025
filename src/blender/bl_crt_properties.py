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
    type: bpy.props.EnumProperty(
        name='CRT Type',
        items=[
            ('DIFFUSE', 'Diffuse', '', 1),
            ('REFLECTIVE', 'Reflective', '', 2),
            ('REFRACTIVE', 'Refractive', '', 3),
            ('CONSTANT', 'Constant', '', 4)
        ]
    )
    smooth_shading: bpy.props.BoolProperty(name='Smooth Shading', default=False)
    ior: bpy.props.FloatProperty(name='IOR', default=1, min=0)
    albedo_texture: bpy.props.PointerProperty(type=bpy.types.Texture, name='Albedo Texture')

    @classmethod
    def register(cls):
        bpy.types.Material.crt = bpy.props.PointerProperty(
            name='CRT Material Properties',
            description='',
            type=cls,
        )
    
    @classmethod
    def unregister(cls):
        del bpy.types.Material.crt


class CRTTextureProperties(bpy.types.PropertyGroup):
    type: bpy.props.EnumProperty(
        name='CRT Type',
        items=[
            ('ALBEDO', 'Albedo', '', 1),
            ('EDGES', 'Edges', '', 2),
            ('CHECKER', 'Checker', '', 3),
            ('BITMAP', 'Bitmap', '', 4),
        ]
    )

    albedo_color: bpy.props.FloatVectorProperty(name='Albedo', subtype='COLOR', min=0, max=1, default=(0.9, 0.9, 0.9))

    edge_color: bpy.props.FloatVectorProperty(name='Edge Color', subtype='COLOR', min=0, max=1, default=(0.9, 0.9, 0.9))
    inner_color: bpy.props.FloatVectorProperty(name='Inner Color', subtype='COLOR', min=0, max=1, default=(0, 0, 0))
    edge_width: bpy.props.FloatProperty(name='Edge Width', default=0.1)

    checker_color_a: bpy.props.FloatVectorProperty(name='Color A', subtype='COLOR', min=0, max=1, default=(0.1, 0.1, 0.1))
    checker_color_b: bpy.props.FloatVectorProperty(name='Color B', subtype='COLOR', min=0, max=1, default=(0.9, 0.9, 0.9))
    square_size: bpy.props.FloatProperty(name='Square Size', default=0.5)

    @classmethod
    def register(cls):
        bpy.types.Texture.crt = bpy.props.PointerProperty(
            name='CRT Texture Properties',
            description='',
            type=cls
        )
    
    @classmethod
    def unregister(cls):
        del bpy.types.Texture.crt
    

classes = (
    CRTRenderSettings,
    CRTMaterialProperties,
    CRTTextureProperties,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
