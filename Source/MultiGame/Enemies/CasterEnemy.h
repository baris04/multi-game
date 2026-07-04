#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "CasterEnemy.generated.h"

class UAbilityComponent;

/**
 * Caster enemy: keeps distance from targets and launches projectile abilities.
 */
UCLASS()
class MULTIGAME_API ACasterEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	ACasterEnemy();

	virtual void PerformAttack(AActor* Target) override;

protected:
	virtual void BeginPlay() override;
	virtual bool WantsToKeepDistance() const override { return true; }

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
	TObjectPtr<UAbilityComponent> AbilityComponent;
};
