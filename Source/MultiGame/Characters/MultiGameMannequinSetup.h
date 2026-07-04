#pragma once

#include "CoreMinimal.h"

class AMultiGameCharacter;
class USkeletalMesh;
class UAnimSequence;
class UMaterialInterface;

namespace MultiGameMannequin
{
	inline const TCHAR* RootPath = TEXT("/Game/MultiGame/Characters/Mannequins");
	inline const TCHAR* AltRootPath = TEXT("/Game/Characters/Mannequins");

	inline const TCHAR* GideonMeshPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Meshes/Gideon.Gideon");
	inline const TCHAR* GideonIdleAnimPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Idle.Idle");
	inline const TCHAR* GideonIdleAnimPathAlt = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/HeroPedestal_Idle.HeroPedestal_Idle");
	inline const TCHAR* GideonMoveAnimPath = TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Jog_Fwd.Jog_Fwd");

	inline const TCHAR* SkeletonMeshPath = TEXT("/Game/Fab/Stylized_Low_Poly_Skeleton/skeleton_model_110.skeleton_model_110");
	inline const TCHAR* SkeletonIdleAnimPath = TEXT("/Game/Fab/Stylized_Low_Poly_Skeleton/skeleton_model_110_Anim.skeleton_model_110_Anim");
	inline const TCHAR* SkeletonMaterialPath = TEXT("/Game/Fab/Stylized_Low_Poly_Skeleton/mat_skeleton.mat_skeleton");

	enum class EVisualPreset : uint8
	{
		Player,
		MeleeEnemy,
		CasterEnemy,
		Boss
	};

	void GetVisualPresetPaths(
		EVisualPreset Preset,
		TArray<FString>& OutMeshPaths,
		TArray<FString>& OutIdleAnimPaths,
		TArray<FString>& OutMoveAnimPaths,
		TArray<FString>& OutMaterialPaths);

	USkeletalMesh* LoadFirstMesh(const TArray<FString>& Paths);
	UAnimSequence* LoadFirstAnimSequence(const TArray<FString>& Paths);
	UMaterialInterface* LoadFirstMaterial(const TArray<FString>& Paths);

	void ApplyVisualPreset(AMultiGameCharacter* Character, EVisualPreset Preset);
	void ConfigureCharacterMesh(
		AMultiGameCharacter* Character,
		USkeletalMesh* BodyMesh,
		UAnimSequence* IdleAnim,
		UMaterialInterface* BodyMaterial = nullptr);
	void ApplyPresetMeshAdjustments(AMultiGameCharacter* Character, EVisualPreset Preset);
	void ApplyMeshTint(class USkeletalMeshComponent* MeshComp, const FLinearColor& TintColor);
}
