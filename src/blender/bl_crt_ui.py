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
        layout.prop(crt_scene, 'refractcontext.ions_on')


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
        layout.prop(mat, 'diffuse_color', text='Albedo Color')
        layout.prop(crt_mat, 'smooth_shading')
        layout.prop(mat, 'use_backface_culling')
    

classes = (
    CRT_RENDER_PT_settings,
    CRT_LIGHT_PT_light,
    CRT_MATERIAL_PT_context_material,
    CRT_MATERIAL_PT_surface,
)


def register():
    for cls in classes:
        bpy.utils.register_class(cls)


def unregister():
    for cls in reversed(classes):
        bpy.utils.unregister_class(cls)
