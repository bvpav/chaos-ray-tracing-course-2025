import json
import math
import os

import bpy
import bmesh
import mathutils
from mathutils import Matrix, Vector
from bpy_extras.io_utils import axis_conversion

_blender_to_rh_conversion = axis_conversion(
    from_forward='-Y', from_up='Z',
    to_forward='Z', to_up='Y'
).to_4x4()

_rh_to_blender_conversion = axis_conversion(
    from_forward='Z', from_up='Y',
    to_forward='-Y', to_up='Z'
).to_4x4()


def _convert_matrix(mat: Matrix) -> list[float]:
    return [v for row in mat.to_3x3().transposed() for v in row]


def _convert_vector(vec: Vector) -> list[float]:
    v = _blender_to_rh_conversion @ Vector((vec.x, vec.y, vec.z, 1.0))
    return [v.x, v.y, v.z]


def _absolute_path(filepath: str) -> str:
    """Return absolute path for external files."""
    if not filepath:
        raise ValueError("Texture has no image file path")
    if bpy.data.is_saved:
        return os.path.abspath(bpy.path.abspath(filepath))
    return os.path.abspath(filepath)


def _ensure_external(image: bpy.types.Image) -> None:
    """Raise if the image is packed or has no external file."""
    if image.packed_file:
        raise ValueError(f"Texture '{image.name}' is packed – external file required")
    if not image.filepath:
        raise ValueError(f"Texture '{image.name}' has no external file")


def build_textures() -> list[dict]:
    """Export all CRT textures with their custom properties."""
    textures = []
    for tex in bpy.data.textures:
        if tex.type != 'IMAGE':
            continue
        if not hasattr(tex, 'crt') or tex.crt is None:
            continue

        crt_tex = tex.crt
        tex_dict = {
            'name': tex.name,
            'type': crt_tex.type.lower()
        }

        if crt_tex.type == 'ALBEDO':
            tex_dict['albedo'] = list(crt_tex.albedo_color)
        elif crt_tex.type == 'EDGES':
            tex_dict['edge_color'] = list(crt_tex.edge_color)
            tex_dict['inner_color'] = list(crt_tex.inner_color)
            tex_dict['edge_width'] = crt_tex.edge_width
        elif crt_tex.type == 'CHECKER':
            tex_dict['color_A'] = list(crt_tex.checker_color_a)
            tex_dict['color_B'] = list(crt_tex.checker_color_b)
            tex_dict['square_size'] = crt_tex.square_size
        elif crt_tex.type == 'BITMAP':
            if tex and tex.image:
                img = tex.image
                _ensure_external(img)
                tex_dict['file_path'] = _absolute_path(img.filepath)
            else:
                raise ValueError(f"Bitmap texture '{tex.name}' has no external image")

        textures.append(tex_dict)
    return textures


def build_settings(scene: bpy.types.Scene) -> dict:
    scale = scene.render.resolution_percentage / 100.0
    width = int(scene.render.resolution_x * scale)
    height = int(scene.render.resolution_y * scale)

    return {
        'background_color': tuple(scene.world.color),
        'image_settings': {
            'width': width,
            'height': height,
            'bucket_size': scene.crt.bucket_size,
        },
        'gi_on': scene.crt.gi_on,
        'reflections_on': scene.crt.reflections_on,
        'refractions_on': scene.crt.refractions_on,
    }


def build_camera(scene: bpy.types.Scene) -> dict:
    cam_obj = scene.camera
    if cam_obj is None:
        raise RuntimeError('No active camera found')

    cam = cam_obj.data
    cam_matrix_rh = _blender_to_rh_conversion @ cam_obj.matrix_world

    return {
        'matrix': _convert_matrix(cam_matrix_rh),
        'position': _convert_vector(cam_obj.matrix_world.translation),
        'fov_degrees': math.degrees(cam.angle),
    }


def build_lights(scene: bpy.types.Scene) -> list[dict]:
    lights = []
    for obj in scene.objects:
        if obj.type == 'LIGHT' and obj.data.type == 'POINT':
            lights.append({
                'intensity': obj.data.energy,
                'position': _convert_vector(obj.matrix_world.translation),
            })
    return lights


def build_materials() -> tuple[list[dict], dict]:
    materials = []
    mat_index_map = {}

    if not bpy.data.materials:
        raise ValueError('Blender file must have at least 1 material')

    for mat in bpy.data.materials:
        if mat.users == 0:
            continue
        mat_index_map[mat] = len(materials)

        material_dict = {
            'type': mat.crt.type.lower(),
            'smooth_shading': mat.crt.smooth_shading,
            'back_face_culling': mat.use_backface_culling,
        }
        if mat.crt.type != 'REFRACTIVE':
            tex = mat.crt.albedo_texture
            if not tex:
                material_dict['albedo'] = *mat.diffuse_color[:3],
            else:
                material_dict['albedo'] = tex.name
        else:
            material_dict['ior'] = mat.crt.ior
        materials.append(material_dict)

    return materials, mat_index_map


def build_objects(depsgraph: bpy.types.Depsgraph, mat_index_map: dict) -> list[dict]:
    objects = []

    for obj in bpy.context.scene.objects:
        if obj.type != 'MESH':
            continue

        eval_obj = obj.evaluated_get(depsgraph)
        mesh = eval_obj.to_mesh()
        if not mesh.loop_triangles:
            mesh.calc_loop_triangles()

        vertex_uv = {}
        if mesh.uv_layers.active:
            uv_layer = mesh.uv_layers.active.data
            for loop in mesh.loops:
                vid = loop.vertex_index
                if vid not in vertex_uv:
                    uv = uv_layer[loop.index].uv
                    vertex_uv[vid] = [uv.x, uv.y, 0.0]

        # Vertices
        vertices = []
        uvs = []
        for v in mesh.vertices:
            pos = eval_obj.matrix_world @ v.co
            vertices.extend(_convert_vector(pos))
            uvs.extend(vertex_uv.get(v.index, [0.0, 0.0, 0.0]))

        # Triangles
        triangles = []
        for tri in mesh.loop_triangles:
            triangles.extend(tri.vertices)

        # Material index
        slot = obj.material_slots[tri.material_index] if obj.material_slots else None
        mat = slot.material if slot else None
        # HACK: If the object has no material, use the first (default) one
        material_index = mat_index_map.get(mat, 0)

        obj_dict = {
            'material_index': material_index,
            'vertices': vertices,
            'triangles': triangles,
        }

        if mesh.uv_layers.active:
            obj_dict['uvs'] = uvs

        objects.append(obj_dict)

        eval_obj.to_mesh_clear()

    return objects


def build_scene_dict(depsgraph: bpy.types.Depsgraph) -> dict:
    materials, mat_index_map = build_materials()

    return {
        'settings': build_settings(depsgraph.scene),
        'camera': build_camera(depsgraph.scene),
        'lights': build_lights(depsgraph.scene),
        'textures': build_textures(),
        'materials': materials,
        'objects': build_objects(depsgraph, mat_index_map),
    }


def _load_image(filepath: str) -> bpy.types.Image:
    """Return an existing image or load a new one."""
    filepath = os.path.normpath(filepath)
    for img in bpy.data.images:
        if os.path.normpath(bpy.path.abspath(img.filepath)) == filepath:
            return img
    return bpy.data.images.load(filepath)


def _ensure_texture(name: str) -> bpy.types.Texture:
    """Return existing texture or create a new one."""
    tex = bpy.data.textures.get(name)
    if tex is None:
        tex = bpy.data.textures.new(name, 'IMAGE')
    return tex


def _ensure_material(name: str) -> bpy.types.Material:
    """Return existing material or create a new one."""
    mat = bpy.data.materials.get(name)
    if mat is None:
        mat = bpy.data.materials.new(name)
    return mat


def _ensure_object(name: str, mesh: bpy.types.Mesh) -> bpy.types.Object:
    """Return existing object or create a new one."""
    obj = bpy.data.objects.get(name)
    if obj is None:
        obj = bpy.data.objects.new(name, mesh)
    else:
        obj.data = mesh
    return obj


def load_textures(textures: list[dict]) -> None:
    for t in textures:
        tex = _ensure_texture(t['name'])
        tex.type = 'IMAGE'
        tex.crt.type = t['type'].upper()

        if t['type'] == 'ALBEDO':
            tex.crt.albedo_color = t['albedo']
        elif t['type'] == 'EDGES':
            tex.crt.edge_color = t['edge_color']
            tex.crt.inner_color = t['inner_color']
            tex.crt.edge_width = t['edge_width']
        elif t['type'] == 'CHECKER':
            tex.crt.checker_color_a = t['color_A']
            tex.crt.checker_color_b = t['color_B']
            tex.crt.square_size = t['square_size']
        elif t['type'] == 'BITMAP':
            img = _load_image(t['file_path'])
            tex.image = img


def load_settings(scene: bpy.types.Scene, settings: dict) -> None:
    s = settings
    scene.world.color = s['background_color']
    if 'bucket_size' in s['image_settings']:
        scene.crt.bucket_size = s['image_settings']['bucket_size']
    if 'gi_on' in s:
        scene.crt.gi_on = s['gi_on']
    if 'reflections_on' in s:
        scene.crt.reflections_on = s['reflections_on']
    if 'refractions_on' in s:
        scene.crt.refractions_on = s['refractions_on']

    scene.render.resolution_x = s['image_settings']['width']
    scene.render.resolution_y = s['image_settings']['height']
    scene.render.resolution_percentage = 100


def load_camera(scene: bpy.types.Scene, camera: dict) -> None:
    cam_obj = scene.camera
    if cam_obj is None:
        cam_data = bpy.data.cameras.new('Camera')
        cam_obj = bpy.data.objects.new('Camera', cam_data)
        scene.collection.objects.link(cam_obj)
        scene.camera = cam_obj
    else:
        cam_data = cam_obj.data

    cam_data.angle = math.radians(camera['fov_degrees'])

    # Build 3x3 matrix from flat list
    m = camera['matrix']
    mat3 = Matrix([
        [m[0], m[1], m[2]],
        [m[3], m[4], m[5]],
        [m[6], m[7], m[8]],
    ])
    mat4 = mat3.transposed().to_4x4()
    cam_obj.matrix_world = _rh_to_blender_conversion @ mat4


def load_lights(scene: bpy.types.Scene, lights: list[dict]) -> None:
    existing = {obj for obj in scene.objects if obj.type == 'LIGHT'}
    for i, l in enumerate(lights):
        name = f'Light_{i}'
        light_data = bpy.data.lights.new(name, 'POINT')
        light_obj = bpy.data.objects.new(name, light_data)
        scene.collection.objects.link(light_obj)
        light_obj.location = _rh_to_blender_conversion @ Vector(l['position'])
        light_data.energy = l['intensity']
    for obj in existing:
        bpy.data.objects.remove(obj, do_unlink=True)


def load_materials(materials: list[dict]) -> dict:
    mat_index_map = {}
    for i, m in enumerate(materials):
        mat = _ensure_material(f'Material_{i}')
        mat_index_map[i] = mat
        mat.crt.type = m['type'].upper()
        mat.crt.smooth_shading = m['smooth_shading']
        mat.use_backface_culling = m.get('back_face_culling', False)

        if m['type'] != 'refractive':
            if isinstance(m['albedo'], str):
                tex = bpy.data.textures.get(m['albedo'])
                mat.crt.albedo_texture = tex
            else:
                mat.diffuse_color = (*m['albedo'], 1.0)
                mat.crt.albedo_texture = None
        else:
            mat.crt.ior = m.get('ior', 1.0)
    return mat_index_map


def load_objects(objects: list[dict], mat_index_map: dict) -> None:
    scene = bpy.context.scene
    existing = {obj for obj in bpy.data.objects if obj.type == 'MESH'}
    for i, o in enumerate(objects):
        mesh = bpy.data.meshes.new(f'Mesh_{i}')
        mesh.vertices.add(len(o['vertices']) // 3)
        mesh.vertices.foreach_set('co', o['vertices'])
        mesh.loops.add(len(o['triangles']))
        mesh.loops.foreach_set('vertex_index', o['triangles'])
        mesh.polygons.add(len(o['triangles']) // 3)
        mesh.polygons.foreach_set('loop_start', range(0, len(o['triangles']), 3))
        mesh.polygons.foreach_set('loop_total', [3] * (len(o['triangles']) // 3))
        mesh.update(calc_edges=True)

        # Convert vertex positions from RH to Blender
        for v in mesh.vertices:
            v.co = _rh_to_blender_conversion @ Vector(v.co)

        if 'uvs' in o:
            uv_layer = mesh.uv_layers.new()
            uvs = o['uvs']
            for loop in mesh.loops:
                vid = loop.vertex_index
                uv_layer.data[loop.index].uv = (uvs[vid * 3], uvs[vid * 3 + 1])

        obj = _ensure_object(f'Object_{i}', mesh)
        if obj.name not in bpy.context.scene.collection.objects:
            bpy.context.scene.collection.objects.link(obj)
        obj.data.materials.clear()
        obj.data.materials.append(mat_index_map[o['material_index']])

    for obj in existing:
        bpy.data.objects.remove(obj, do_unlink=True)


def load_scene_dict(scene_dict: dict) -> None:
    scene = bpy.context.scene
    load_settings(scene, scene_dict['settings'])
    load_camera(scene, scene_dict['camera'])
    load_lights(scene, scene_dict['lights'])
    if 'textures' in scene_dict:
        load_textures(scene_dict['textures'])
    mat_index_map = load_materials(scene_dict['materials'])
    load_objects(scene_dict['objects'], mat_index_map)
