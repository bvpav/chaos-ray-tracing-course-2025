import bpy

class CRTButtonsPanel:
    bl_space_type = 'PROPERTIES'
    bl_region_type = 'WINDOW'
    COMPAT_ENGINES = {'CRT'}

    @classmethod
    def poll(cls, context):
        return context.engine in cls.COMPAT_ENGINES


class CRT_RENDER_PT_settings(CRTButtonsPanel, bpy.types.Panel):
    bl_label = 'CRT Settings'
    bl_context = 'render'

    def draw(self, context):
        layout = self.layout

        scene = context.scene
        crt_scene = scene.crt

        layout.use_property_split = True
        layout.use_property_decorate = False

        layout.prop(crt_scene, 'bucket_size')
        layout.prop(crt_scene, 'gi_on')
        layout.prop(crt_scene, 'reflections_on')
        layout.prop(crt_scene, 'refractions_on')

        layout.separator()

        layout.prop(crt_scene, 'max_ray_depth')
        layout.prop(crt_scene, 'diffuse_reflection_ray_count')
        layout.prop(crt_scene, 'shadow_bias')
        layout.prop(crt_scene, 'reflection_bias')
        layout.prop(crt_scene, 'diffuse_reflection_bias')
        layout.prop(crt_scene, 'refraction_bias')


class CRT_LIGHT_PT_light(CRTButtonsPanel, bpy.types.Panel):
    bl_label = 'CRT Light'
    bl_context = 'data'

    @classmethod
    def poll(cls, context):
        return context.light and CRTButtonsPanel.poll(context)
    
    def draw(self, context):
        layout = self.layout

        light = context.light
        is_point = light.type == 'POINT'

        layout.use_property_split = True

        if not is_point:
            self.layout.label(text=f'{light.type.title()} lights are not supported', icon='ERROR')

        row = layout.row()
        row.enabled = is_point
        row.prop(light, 'energy', text='Intensity')


class CRT_MATERIAL_PT_context_material(CRTButtonsPanel, bpy.types.Panel):
    bl_label = ''
    bl_context = 'material'
    bl_options = {'HIDE_HEADER'}

    @classmethod
    def poll(cls, context):
        return (context.material or context.object) and CRTButtonsPanel.poll(context)

    def draw(self, context):
        layout = self.layout

        layout.use_property_split = True

        mat = context.material
        ob = context.object
        space = context.space_data

        if ob:
            layout.template_ID(ob, 'active_material', new='material.new')
        elif mat:
            layout.template_ID(space, 'pin_id')


class CRT_MATERIAL_PT_surface(CRTButtonsPanel, bpy.types.Panel):
    bl_label = 'CRT Material'
    bl_context = 'material'

    @classmethod
    def poll(cls, context):
        return context.material and CRTButtonsPanel.poll(context)
    
    def draw(self, context):
        layout = self.layout
        mat = context.material
        crt_mat = mat.crt

        layout.use_property_split = True
        layout.prop(crt_mat, 'type')
        if crt_mat.type != 'REFRACTIVE':
            row = layout.row()
            row.enabled = crt_mat.albedo_texture is None
            row.prop(mat, 'diffuse_color', text='Albedo Color')
        else:
            layout.prop(crt_mat, 'ior')

        layout.separator()

        layout.prop(crt_mat, 'smooth_shading')
        layout.prop(mat, 'use_backface_culling')


class CRT_MATERIAL_PT_albedo_texture(CRTButtonsPanel, bpy.types.Panel):
    bl_label = 'Albedo Texture'
    bl_context = 'material'
    bl_parent_id = CRT_MATERIAL_PT_surface.__name__

    @classmethod
    def poll(cls, context):
        return CRTButtonsPanel.poll(context) and context.material and context.material.crt.type != 'REFRACTIVE'

    def draw(self, context):
        layout = self.layout

        mat = context.material
        crt_mat = mat.crt
        tex = crt_mat.albedo_texture

        layout.use_property_split = True

        layout.template_ID(crt_mat, 'albedo_texture', new='texture.new')
        layout.separator()

        if tex:
            crt_tex = tex.crt

            layout.prop(crt_tex, 'type')

            col = layout.column()
            if crt_tex.type == 'ALBEDO':
                col.prop(crt_tex, 'albedo_color')
            elif crt_tex.type == 'EDGES':
                col.prop(crt_tex, 'edge_color')
                col.prop(crt_tex, 'inner_color')
                col.prop(crt_tex, 'edge_width')
            elif crt_tex.type == 'CHECKER':
                col = layout.column()
                col.prop(crt_tex, 'checker_color_a')
                col.prop(crt_tex, 'checker_color_b')
                col.prop(crt_tex, 'square_size')
            elif crt_tex.type == 'BITMAP':
                col.template_ID(tex, 'image', open='image.open')
    

classes = (
    CRT_RENDER_PT_settings,
    CRT_LIGHT_PT_light,
    CRT_MATERIAL_PT_context_material,
    CRT_MATERIAL_PT_surface,
    CRT_MATERIAL_PT_albedo_texture,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
