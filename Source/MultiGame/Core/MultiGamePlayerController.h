#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Core/MultiGameTypes.h"
#include "MultiGamePlayerController.generated.h"

/**
 * Routes character-creation choices to the server and exposes UI hooks that the
 * HUD/menu Blueprints implement.
 */
UCLASS()
class MULTIGAME_API AMultiGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMultiGamePlayerController();

	/** Send the locally chosen appearance to the server (authoritative PlayerState). */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MultiGame")
	void ServerSetAppearance(const FCharacterAppearance& NewAppearance);

	/** Toggle lobby-ready state on the server. */
	UFUNCTION(BlueprintCallable, Server, Reliable, Category = "MultiGame")
	void ServerSetReady(bool bReady);

	/** Creates and shows the C++ gameplay HUD widget on the local client. */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|UI")
	void ShowGameplayHUD();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnRep_Pawn() override;

	void SendAppearanceToServer();

	/** HUD widget class (defaults to the C++ UHUDWidget; overridable in BP). */
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;

	UPROPERTY()
	TObjectPtr<class UUserWidget> HUDWidget;

	bool bAppearanceSent = false;
	bool bHUDShown = false;
};
