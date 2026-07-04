#pragma once

#include "CoreMinimal.h"
#include "Enemies/EnemyBase.h"
#include "BossCharacter.generated.h"

class UCombatComponent;
class UAbilityComponent;
class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBossPhaseChanged, int32, NewPhase);

/**
 * Multi-phase boss. Phases trigger at health percentage thresholds and change the
 * boss's attack cadence and available abilities (melee combo, AOE, projectile waves).
 */
UCLASS()
class MULTIGAME_API ABossCharacter : public AEnemyBase
{
	GENERATED_BODY()

public:
	ABossCharacter();

	virtual void PerformAttack(AActor* Target) override;

	UPROPERTY(BlueprintAssignable, Category = "Boss")
	FOnBossPhaseChanged OnBossPhaseChanged;

	UFUNCTION(BlueprintPure, Category = "Boss")
	int32 GetCurrentPhase() const { return CurrentPhase; }

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleHealthChanged(UHealthComponent* HealthComp, float Health, float MaxHealth, AActor* InstigatorActor);

	void EnterPhase(int32 NewPhase);

	/** Fires a ring/wave of projectiles around the boss (AOE-style attack). */
	void CastProjectileWave(int32 ProjectileCount);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss|Components")
	TObjectPtr<UAbilityComponent> AbilityComponent;

	/** Health-percent thresholds (descending, e.g. {0.66, 0.33}) that advance phases. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TArray<float> PhaseHealthThresholds = { 0.66f, 0.33f };

	/** Attack cooldown per phase; index 0 = phase 1. Later phases attack faster. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TArray<float> PhaseAttackCooldowns = { 4.f, 3.f, 2.5f };

	/** Projectiles in the wave per phase (0 disables waves in that phase). */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TArray<int32> PhaseWaveProjectiles = { 0, 4, 6 };

	/** Projectile class used for wave attacks. */
	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	TSubclassOf<class AProjectileBase> WaveProjectileClass;

	UPROPERTY(EditDefaultsOnly, Category = "Boss")
	float WaveProjectileDamage = 4.f;

	int32 CurrentPhase = 0;
	bool bMeleeToggle = false;
};
