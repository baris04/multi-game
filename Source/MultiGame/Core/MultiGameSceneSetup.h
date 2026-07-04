#pragma once

#include "CoreMinimal.h"

class UWorld;

/** Spawns / boosts sun, sky, fog and post-process so maps are bright without editor setup. */
namespace MultiGameSceneSetup
{
	void ApplyGameplayLighting(UWorld* World);

	/** Decorative arena props: pillars, torches, center platform, boundary ring. */
	void ApplyArenaDecoration(UWorld* World, float ArenaHalfSize);

	/** Top of the procedural arena floor (capsule center = this + half height). */
	inline constexpr float ArenaFloorTopZ = 0.f;
	inline constexpr float DefaultCharacterCapsuleHalfHeight = 96.f;

	inline float GetCharacterSpawnZ()
	{
		return ArenaFloorTopZ + DefaultCharacterCapsuleHalfHeight + 4.f;
	}
}
