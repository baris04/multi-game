import unreal

MAP_PATHS = [
    "/Game/MultiGame/Maps/MainMenu",
    "/Game/MultiGame/Maps/Arena",
]

for map_path in MAP_PATHS:
    unreal.log(f"Updating lighting settings: {map_path}")
    unreal.EditorLoadingAndSavingUtils.load_map(map_path)

    editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
    world = editor.get_editor_world()
    if not world:
        unreal.log_error(f"No world for {map_path}")
        continue

    world_settings = world.get_world_settings()
    world_settings.set_editor_property("force_no_precomputed_lighting", True)

    unreal.EditorLoadingAndSavingUtils.save_current_level()
    unreal.log(f"Saved {map_path} with dynamic-only lighting.")

unreal.log("FixMapLighting.py finished.")
