"""
Import mannequin assets into the project.

OPTION A - Editor (recommended):
  1. Epic Launcher -> Add "Third Person" template content to library OR use Fab "UE5 Mannequins"
  2. Migrate / copy the whole "Mannequins" folder into:
       Content/MultiGame/Characters/Mannequins
  3. Required files (names must match):
       Meshes/SKM_Manny, Meshes/SKM_Quinn
       Animations/ABP_Manny, Animations/ABP_Quinn
       Materials/Instances/Manny/MI_Manny_01, MI_Manny_02
       Materials/Instances/Quinn/MI_Quinn_01
  4. Save all, restart editor, Play.

OPTION B - Auto copy from UE install (this script):
  Tools -> Execute Python Script -> run this file.
"""
import shutil
import unreal
from pathlib import Path

TEMPLATE_MANNEQUINS = Path(
    r"C:/Program Files/Epic Games/UE_5.5/Templates/TemplateResources/High/Characters/Content/Mannequins"
)
PROJECT_ROOT = Path(unreal.Paths.project_content_dir())
DEST = PROJECT_ROOT / "MultiGame" / "Characters" / "Mannequins"

if not TEMPLATE_MANNEQUINS.exists():
    unreal.log_error(f"Template mannequins not found: {TEMPLATE_MANNEQUINS}")
    unreal.log_error("Import manually to Content/MultiGame/Characters/Mannequins")
else:
    DEST.parent.mkdir(parents=True, exist_ok=True)
    if DEST.exists():
        unreal.log(f"Mannequins folder already exists: {DEST}")
    else:
        shutil.copytree(TEMPLATE_MANNEQUINS, DEST)
        unreal.log(f"Copied mannequins to {DEST}")

    unreal.EditorAssetLibrary.save_directory("/Game/MultiGame/Characters/Mannequins", only_if_is_dirty=False, recursive=True)
    unreal.log("Done. C++ will auto-load on BeginPlay.")
