import json
import math

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


def build_settings() -> dict:
    return {
        'background_color': [0, 0, 0],
        'image_settings': {
            'width': 1080,
            'height': 1080,
            'bucket_size': 24,
        },
        'gi_on': True,
        'reflections_on': True,
        'refractions_on': True,
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
        'fov_degrees': math.degrees(cam.angle_x),
    }


def build_lights(scene: bpy.types.Scene) -> list[dict]:
    lights = []
    for obj in scene.objects:
        if obj.type == 'LIGHT' and obj.data.type == 'POINT':
            lights.append({
                'intensity': obj.data.energy,
                'position': _convert_vector(obj.matrix_world.translation),
            })
    # TODO: support other light types
    return lights


def build_materials() -> tuple[list[dict], dict]:
    materials = []
    mat_index_map = {}

    if not bpy.data.materials:
        raise ValueError("Blender file must have at least 1 material")

    for mat in bpy.data.materials:
        if mat.users == 0:
            continue
        mat_index_map[mat] = len(materials)

        materials.append({
            'type': 'diffuse',
            'albedo': list(mat.diffuse_color[:3]),
            'smooth_shading': not mat.use_backface_culling,
            'back_face_culling': mat.use_backface_culling,
        })
        # TODO: detect reflective / refractive / constant materials

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

        # Vertices
        vertices = []
        for v in mesh.vertices:
            pos = eval_obj.matrix_world @ v.co
            vertices.extend(_convert_vector(pos))

        # Triangles
        triangles = []
        for tri in mesh.loop_triangles:
            triangles.extend(tri.vertices)

        # Material index
        slot = obj.material_slots[tri.material_index] if obj.material_slots else None
        mat = slot.material if slot else None
        material_index = mat_index_map.get(mat, 0)

        objects.append({
            'material_index': material_index,
            'vertices': vertices,
            'triangles': triangles,
        })

        eval_obj.to_mesh_clear()

    return objects


def build_scene_dict(depsgraph: bpy.types.Depsgraph) -> dict:
    materials, mat_index_map = build_materials()

    return {
        'settings': build_settings(),
        'camera': build_camera(depsgraph.scene),
        'lights': build_lights(depsgraph.scene),
        'materials': materials,
        'objects': build_objects(depsgraph, mat_index_map),
    }
