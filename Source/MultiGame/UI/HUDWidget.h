#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Core/MultiGameState.h"
#include "HUDWidget.generated.h"

class UHealthComponent;
class UProgressBar;
class UTextBlock;

/**
 * Gameplay HUD built in C++ (no WBP): player HP bar, wave text, boss HP bar,
 * and a big status message for win/lose.
 */
UCLASS()
class MULTIGAME_API UHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetLocalHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	float GetBossHealthPercent() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	bool IsBossActive() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	int32 GetCurrentWave() const;

	UFUNCTION(BlueprintPure, Category = "HUD")
	EMatchPhase GetMatchPhase() const;

protected:
	UHealthComponent* GetLocalHealthComponent() const;

	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TObjectPtr<UProgressBar> BossBar;

	UPROPERTY()
	TObjectPtr<UTextBlock> WaveText;

	UPROPERTY()
	TObjectPtr<UTextBlock> StatusText;

	UPROPERTY()
	TObjectPtr<UTextBlock> HelpText;

	UPROPERTY()
	TObjectPtr<UWidget> BossContainer;
};
