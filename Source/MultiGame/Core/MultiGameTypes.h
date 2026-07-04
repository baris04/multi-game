#pragma once

#include "CoreMinimal.h"
#include "MultiGameTypes.generated.h"

UENUM(BlueprintType)
enum class ECharacterClass : uint8
{
	Warrior		UMETA(DisplayName = "Warrior"),
	Mage		UMETA(DisplayName = "Mage"),
	Rogue		UMETA(DisplayName = "Rogue")
};

UENUM(BlueprintType)
enum class ETeam : uint8
{
	Players		UMETA(DisplayName = "Players"),
	Enemies		UMETA(DisplayName = "Enemies")
};

/** Replicated visual appearance chosen during character creation. */
USTRUCT(BlueprintType)
struct FCharacterAppearance
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString CharacterName = TEXT("Hero");

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECharacterClass CharacterClass = ECharacterClass::Warrior;

	/** Index into the body mesh options list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BodyMeshIndex = 0;

	/** Index into the head/hair mesh options list. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 HeadMeshIndex = 0;

	/** Primary tint applied to the body material (skin / armor). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor PrimaryColor = FLinearColor::White;

	/** Secondary tint applied to the body material (cloth / detail). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor SecondaryColor = FLinearColor::Gray;
};
