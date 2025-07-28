if '_needs_reload' in locals():
    from importlib import reload as _reload
    for _mod in (
        '_crt',
        'bl_crt_engine',
        'bl_crt_json',
    ):
        try:
            _reload(locals()[_mod])
        except KeyError:
            pass

_needs_reload = True


def register():
    from . import bl_crt_engine

    bl_crt_engine.register()


def unregister():
    from . import bl_crt_engine

    bl_crt_engine.unregister()
