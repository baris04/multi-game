#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MenuGameMode.generated.h"

/**
 * Lightweight game mode for the MainMenu map: no pawns, just shows the C++ menu.
 */
UCLASS()
class MULTIGAME_API AMenuGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMenuGameMode();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Menu")
	TSubclassOf<class UUserWidget> MenuWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> MenuWidget;
};
