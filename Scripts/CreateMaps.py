import unreal

MAP_PATHS = [
    "/Game/MultiGame/Maps/MainMenu",
    "/Game/MultiGame/Maps/Arena",
]

level_editor = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)

for map_path in MAP_PATHS:
    unreal.log(f"Creating map: {map_path}")
    success = level_editor.new_level(map_path)
    unreal.log(f"Created {map_path}: {success}")

unreal.log("CreateMaps.py finished.")
