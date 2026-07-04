#include "Characters/MultiGameMannequinSetup.h"
#include "Characters/MultiGameCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	void AddPath(TArray<FString>& Paths, const FString& Path)
	{
		if (!Path.IsEmpty())
		{
			Paths.AddUnique(Path);
		}
	}

	FString MeshPath(const TCHAR* Root, const TCHAR* Name)
	{
		return FString::Printf(TEXT("%s/Meshes/%s.%s"), Root, Name, Name);
	}

	FString MannequinAnimPath(const TCHAR* Root, const TCHAR* Folder, const TCHAR* Name)
	{
		return FString::Printf(TEXT("%s/Animations/%s/%s.%s"), Root, Folder, Name, Name);
	}

	FString MaterialPath(const TCHAR* Root, const TCHAR* Folder, const TCHAR* Name)
	{
		return FString::Printf(TEXT("%s/Materials/Instances/%s/%s.%s"), Root, Folder, Name, Name);
	}

	template <typename TObjectType>
	TObjectType* LoadFirst(const TArray<FString>& Paths)
	{
		for (const FString& Path : Paths)
		{
			if (TObjectType* Asset = LoadObject<TObjectType>(nullptr, *Path))
			{
				return Asset;
			}
		}
		return nullptr;
	}
}

void MultiGameMannequin::GetVisualPresetPaths(
	EVisualPreset Preset,
	TArray<FString>& OutMeshPaths,
	TArray<FString>& OutIdleAnimPaths,
	TArray<FString>& OutMoveAnimPaths,
	TArray<FString>& OutMaterialPaths)
{
	OutMeshPaths.Reset();
	OutIdleAnimPaths.Reset();
	OutMoveAnimPaths.Reset();
	OutMaterialPaths.Reset();

	const TCHAR* Roots[] = { RootPath, AltRootPath };

	switch (Preset)
	{
	case EVisualPreset::Player:
		AddPath(OutMeshPaths, GideonMeshPath);
		AddPath(OutIdleAnimPaths, GideonIdleAnimPath);
		AddPath(OutIdleAnimPaths, GideonIdleAnimPathAlt);
		AddPath(OutMoveAnimPaths, GideonMoveAnimPath);
		for (const TCHAR* Root : Roots)
		{
			AddPath(OutMeshPaths, MeshPath(Root, TEXT("SKM_Manny")));
			AddPath(OutIdleAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Idle")));
			AddPath(OutMoveAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Walk_Fwd")));
			AddPath(OutMoveAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Run_Fwd")));
			AddPath(OutMaterialPaths, MaterialPath(Root, TEXT("Manny"), TEXT("MI_Manny_01")));
		}
		break;

	case EVisualPreset::MeleeEnemy:
		AddPath(OutMeshPaths, GideonMeshPath);
		AddPath(OutIdleAnimPaths, GideonIdleAnimPath);
		AddPath(OutIdleAnimPaths, GideonIdleAnimPathAlt);
		AddPath(OutMoveAnimPaths, GideonMoveAnimPath);
		for (const TCHAR* Root : Roots)
		{
			AddPath(OutMeshPaths, MeshPath(Root, TEXT("SKM_Manny")));
			AddPath(OutIdleAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Idle")));
			AddPath(OutMoveAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Walk_Fwd")));
		}
		break;

	case EVisualPreset::CasterEnemy:
	case EVisualPreset::Boss:
		AddPath(OutMeshPaths, SkeletonMeshPath);
		AddPath(OutIdleAnimPaths, SkeletonIdleAnimPath);
		AddPath(OutMoveAnimPaths, SkeletonIdleAnimPath);
		AddPath(OutMaterialPaths, SkeletonMaterialPath);
		for (const TCHAR* Root : Roots)
		{
			AddPath(OutMeshPaths, MeshPath(Root, TEXT("SKM_Manny")));
			AddPath(OutIdleAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Idle")));
			AddPath(OutMoveAnimPaths, MannequinAnimPath(Root, TEXT("Manny"), TEXT("MM_Walk_Fwd")));
		}
		break;

	default:
		break;
	}
}

USkeletalMesh* MultiGameMannequin::LoadFirstMesh(const TArray<FString>& Paths)
{
	return LoadFirst<USkeletalMesh>(Paths);
}

UAnimSequence* MultiGameMannequin::LoadFirstAnimSequence(const TArray<FString>& Paths)
{
	return LoadFirst<UAnimSequence>(Paths);
}

UMaterialInterface* MultiGameMannequin::LoadFirstMaterial(const TArray<FString>& Paths)
{
	return LoadFirst<UMaterialInterface>(Paths);
}

void MultiGameMannequin::ApplyVisualPreset(AMultiGameCharacter* Character, EVisualPreset Preset)
{
	if (!Character)
	{
		return;
	}

	TArray<FString> MeshPaths;
	TArray<FString> IdleAnimPaths;
	TArray<FString> MoveAnimPaths;
	TArray<FString> MaterialPaths;
	GetVisualPresetPaths(Preset, MeshPaths, IdleAnimPaths, MoveAnimPaths, MaterialPaths);
	Character->SetCharacterVisualAssetPaths(MeshPaths, IdleAnimPaths, MoveAnimPaths, MaterialPaths);
	Character->SetVisualPreset(Preset);
}

void MultiGameMannequin::ConfigureCharacterMesh(
	AMultiGameCharacter* Character,
	USkeletalMesh* BodyMesh,
	UAnimSequence* IdleAnim,
	UMaterialInterface* BodyMaterial)
{
	if (!Character || !BodyMesh)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	MeshComp->SetSkeletalMesh(BodyMesh);

	if (IdleAnim)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComp->PlayAnimation(IdleAnim, true);
	}

	if (BodyMaterial)
	{
		const int32 NumMaterials = MeshComp->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
		{
			MeshComp->SetMaterial(MaterialIndex, BodyMaterial);
		}
	}

	if (UStaticMeshComponent* Placeholder = Character->GetPlaceholderMesh())
	{
		Placeholder->SetVisibility(false, true);
	}
}

void MultiGameMannequin::ApplyPresetMeshAdjustments(AMultiGameCharacter* Character, EVisualPreset Preset)
{
	if (!Character)
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = Character->GetMesh();
	if (!MeshComp)
	{
		return;
	}

	switch (Preset)
	{
	case EVisualPreset::Player:
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -96.f));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		MeshComp->SetRelativeScale3D(FVector(1.f));
		break;

	case EVisualPreset::MeleeEnemy:
	case EVisualPreset::CasterEnemy:
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		MeshComp->SetRelativeScale3D(FVector(1.15f));
		break;

	case EVisualPreset::Boss:
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -95.f));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
		MeshComp->SetRelativeScale3D(FVector(1.45f));
		break;

	default:
		break;
	}
}

void MultiGameMannequin::ApplyMeshTint(USkeletalMeshComponent* MeshComp, const FLinearColor& TintColor)
{
	if (!MeshComp)
	{
		return;
	}

	const int32 NumMaterials = MeshComp->GetNumMaterials();
	for (int32 MaterialIndex = 0; MaterialIndex < NumMaterials; ++MaterialIndex)
	{
		if (UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(MaterialIndex))
		{
			MID->SetVectorParameterValue(TEXT("Color"), TintColor);
			MID->SetVectorParameterValue(TEXT("BaseColor"), TintColor);
			MID->SetVectorParameterValue(TEXT("Tint"), TintColor);
		}
	}
}
