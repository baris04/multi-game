#pragma once

#include "CoreMinimal.h"

/** Short-lived glowing spheres for melee swings and ability flashes (no debug wireframes). */
class MULTIGAME_API FCombatVisuals
{
public:
	static void SpawnGlowSphere(UWorld* World, const FVector& Location, float Radius, const FLinearColor& Color, float LifeSeconds = 0.28f);

	static void ApplyGlowMaterial(class UMeshComponent* MeshComp, const FLinearColor& Color, float EmissiveScale = 6.f);
};
