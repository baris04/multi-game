#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHealthBarWidget.generated.h"

class UHealthComponent;
class UProgressBar;

/**
 * World-space health bar shown above enemies. Built entirely in C++ (no WBP asset).
 * Follows the health component assigned via SetHealthComponent().
 */
UCLASS()
class MULTIGAME_API UEnemyHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void SetHealthComponent(UHealthComponent* InHealthComponent);

	/** Sets the fill colour so different enemy types can be distinguished. */
	void SetBarColor(const FLinearColor& Color);

protected:
	UPROPERTY()
	TObjectPtr<UProgressBar> HealthBar;

	UPROPERTY()
	TWeakObjectPtr<UHealthComponent> HealthComponent;

	FLinearColor BarColor = FLinearColor(0.9f, 0.15f, 0.1f);
};
