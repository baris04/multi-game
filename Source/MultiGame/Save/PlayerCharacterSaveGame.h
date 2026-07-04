#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Core/MultiGameTypes.h"
#include "PlayerCharacterSaveGame.generated.h"

/**
 * Persists the local player's character creation choices to disk.
 */
UCLASS()
class MULTIGAME_API UPlayerCharacterSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
	FCharacterAppearance Appearance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
	FString SlotName = TEXT("PlayerCharacter");

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "SaveGame")
	int32 UserIndex = 0;
};
