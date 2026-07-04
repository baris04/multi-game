#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AbilityComponent.generated.h"

class AMultiGameCharacter;
class AProjectileBase;
class UAnimMontage;

/** Data describing a single castable ability (projectile-based). */
USTRUCT(BlueprintType)
struct FAbilityDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName AbilityId = TEXT("Fireball");

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TSubclassOf<AProjectileBase> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UAnimMontage> CastMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Cooldown = 3.f;

	/** Delay from cast start to projectile spawn (matches the cast animation). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CastTime = 0.4f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName MuzzleSocketName = TEXT("hand_r");
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAbilityCooldownChanged, int32, AbilityIndex, float, RemainingSeconds);

/**
 * Casts cooldown-gated projectile abilities. Shared by player mages and caster enemies.
 * Spawning happens on the server; the cast montage plays on all clients.
 */
UCLASS(ClassGroup = (MultiGame), meta = (BlueprintSpawnableComponent))
class MULTIGAME_API UAbilityComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UAbilityComponent();

	UPROPERTY(BlueprintAssignable, Category = "Ability")
	FOnAbilityCooldownChanged OnAbilityCooldownChanged;

	/** Server-side: attempt to cast the ability at the given index toward a target/direction. */
	UFUNCTION(BlueprintCallable, Category = "Ability")
	bool TryCastAbility(int32 AbilityIndex, FVector TargetLocation);

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsOnCooldown(int32 AbilityIndex) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	float GetCooldownRemaining(int32 AbilityIndex) const;

	UFUNCTION(BlueprintPure, Category = "Ability")
	int32 GetAbilityCount() const { return Abilities.Num(); }

	/** Ensures at least one projectile ability exists (no Blueprint setup required). */
	void EnsureDefaultProjectileAbility();

	/** Override ability damage/cooldown (e.g. weaker enemy bolts). */
	void ConfigureAbility(int32 AbilityIndex, float Damage, float Cooldown);

protected:
	virtual void BeginPlay() override;

	void FinishCast(int32 AbilityIndex, FVector TargetLocation);

	UPROPERTY(EditDefaultsOnly, Category = "Ability")
	TArray<FAbilityDefinition> Abilities;

	UPROPERTY()
	TObjectPtr<AMultiGameCharacter> OwnerCharacter;

	/** World-time at which each ability becomes ready again (index-aligned). */
	TArray<double> CooldownEndTimes;

	bool bIsCasting = false;

	FTimerHandle CastTimer;
};
