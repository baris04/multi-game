#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MultiGameTypes.h"
#include "CharacterCreationWidget.generated.h"

/**
 * Base class for WBP_CharacterCreation. Edits a working appearance and stores it
 * on the GameInstance (carried into the match) plus an optional disk save.
 */
UCLASS(Abstract)
class MULTIGAME_API UCharacterCreationWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Character")
	FCharacterAppearance WorkingAppearance;

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterName(const FString& InName) { WorkingAppearance.CharacterName = InName; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetCharacterClass(ECharacterClass InClass) { WorkingAppearance.CharacterClass = InClass; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetBodyMeshIndex(int32 InIndex) { WorkingAppearance.BodyMeshIndex = InIndex; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetHeadMeshIndex(int32 InIndex) { WorkingAppearance.HeadMeshIndex = InIndex; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetPrimaryColor(FLinearColor InColor) { WorkingAppearance.PrimaryColor = InColor; }

	UFUNCTION(BlueprintCallable, Category = "Character")
	void SetSecondaryColor(FLinearColor InColor) { WorkingAppearance.SecondaryColor = InColor; }

	/** Commit the working appearance to the GameInstance and save to disk. */
	UFUNCTION(BlueprintCallable, Category = "Character")
	void ConfirmAppearance();

	/** Preview hook: implement in Blueprint to update a preview actor/mesh. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Character")
	void OnAppearancePreviewUpdated();
};
