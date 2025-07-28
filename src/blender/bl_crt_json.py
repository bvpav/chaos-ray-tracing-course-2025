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
    to_forward='Z',    to_up='Y'
).to_4x4()

_rh_to_blender_conversion = axis_conversion(
    from_forward='Z', from_up='Y',
    to_forward='-Y', to_up='Z'
).to_4x4()


def _convert_matrix(mat: Matrix) -> list[float]:
    return [v for row in mat.to_3x3() for v in row]


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
            print(material_dict['albedo'])
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
