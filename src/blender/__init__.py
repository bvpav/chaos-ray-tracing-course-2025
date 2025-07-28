if '_needs_reload' in locals():
    import importlib as _importlib
    import sys as _sys
    for _name in list(_sys.modules):
        if _name.startswith(__package__ + '.'):
            _importlib.reload(_sys.modules[_name])

_needs_reload = True


def register():
    from . import bl_crt_engine
    from . import bl_crt_properties
    from . import bl_crt_ui
    from . import bl_crt_ops

    bl_crt_properties.register()
    bl_crt_engine.register()
    bl_crt_ui.register()
    bl_crt_ops.register()


def unregister():
    from . import bl_crt_engine
    from . import bl_crt_properties
    from . import bl_crt_ui
    from . import bl_crt_ops

    bl_crt_ops.unregister()
    bl_crt_engine.unregister()
    bl_crt_ui.unregister()
    bl_crt_properties.unregister()
