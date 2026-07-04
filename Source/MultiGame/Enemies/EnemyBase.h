#pragma once

#include "CoreMinimal.h"
#include "Characters/MultiGameCharacter.h"
#include "EnemyBase.generated.h"

class UCombatComponent;
class UAbilityComponent;
class UWidgetComponent;

/**
 * Base AI enemy. Subclasses define how they attack (melee vs caster) and their
 * preferred engagement distance so the shared AI controller can position them.
 */
UCLASS(Abstract)
class MULTIGAME_API AEnemyBase : public AMultiGameCharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	/** Server-side: execute this enemy's attack against the given target. */
	virtual void PerformAttack(AActor* Target);

	/** How close (uu) the AI tries to stay to its target. Melee small, caster large. */
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	virtual float GetPreferredDistance() const { return PreferredDistance; }

	/** Max distance an attack can land / be started. */
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	virtual float GetAttackRange() const { return AttackRange; }

	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	float GetAttackCooldown() const { return AttackCooldown; }

	/** Casters want to keep distance and retreat if the target gets too close. */
	UFUNCTION(BlueprintPure, Category = "Enemy|AI")
	virtual bool WantsToKeepDistance() const { return false; }

protected:
	virtual void BeginPlay() override;
	virtual void OnDeathEffects() override;

	/** Colour of this enemy's floating health bar (overridden per subclass). */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|UI")
	FLinearColor HealthBarColor = FLinearColor(0.9f, 0.15f, 0.1f);

	/** World-space health bar shown above the enemy. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy|Components")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComp;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI")
	float PreferredDistance = 150.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI")
	float AttackRange = 200.f;

	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI")
	float AttackCooldown = 2.f;

	/** Seconds the ragdoll corpse remains before being cleaned up. */
	UPROPERTY(EditDefaultsOnly, Category = "Enemy|AI")
	float CorpseLifeSpan = 6.f;
};
