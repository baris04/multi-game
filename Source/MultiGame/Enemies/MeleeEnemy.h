#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "MeleeEnemy.generated.h"

class UCombatComponent;

/**
 * Melee enemy: closes to short range and swings via the combat component.
 */
UCLASS()
class MULTIGAME_API AMeleeEnemy : public AEnemyBase
{
	GENERATED_BODY()

public:
	AMeleeEnemy();

	virtual void PerformAttack(AActor* Target) override;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
	TObjectPtr<UCombatComponent> CombatComponent;
};
