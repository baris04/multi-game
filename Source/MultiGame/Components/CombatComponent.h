#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AMultiGameCharacter;
class UAnimMontage;
class UAnimSequence;
class UDamageType;

/**
 * Melee combat: light-attack combo chain + heavy attack, with an animation-driven
 * damage window that sphere-traces for targets. Server-authoritative damage.
 */
UCLASS(ClassGroup = (MultiGame), meta = (BlueprintSpawnableComponent))
class MULTIGAME_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Called on the server to request a light attack (combo aware). */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestLightAttack();

	/** Called on the server to request a heavy attack. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void RequestHeavyAttack();

	/** Opened/closed by the melee hitbox anim notify to enable trace damage. */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void OpenDamageWindow();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void CloseDamageWindow();

	UFUNCTION(BlueprintPure, Category = "Combat")
	bool IsAttacking() const { return bIsAttacking; }

	void SetDamageValues(float Light, float Heavy);

protected:
	virtual void BeginPlay() override;

	void PerformAttack(UAnimMontage* Montage, FName Section, float InDamage);
	void PerformAnimatedMelee(UAnimSequence* Animation, float InDamage);
	void LoadDefaultAttackAnimations();
	/** Melee hit without animation assets (prototype fallback). */
	void PerformInstantMelee(float InDamage);
	void PerformMeleeSweep();
	FVector GetMeleeForward() const;
	void PerformTrace();
	void ResetCombo();

	UPROPERTY()
	TObjectPtr<AMultiGameCharacter> OwnerCharacter;

	/** One montage per combo step; combo advances while pressed within the window. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Montages")
	TArray<TObjectPtr<UAnimMontage>> LightComboMontages;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Montages")
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animations")
	TArray<TObjectPtr<UAnimSequence>> LightAttackAnimations;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Animations")
	TObjectPtr<UAnimSequence> HeavyAttackAnimation;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	float LightDamage = 15.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	float HeavyDamage = 35.f;

	/** Socket on the weapon/hand mesh the trace originates from. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	FName TraceSocketName = TEXT("hand_r");

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	float TraceRadius = 100.f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Trace")
	float TraceReach = 280.f;

	/** Seconds after an attack the next combo input is still accepted. */
	UPROPERTY(EditDefaultsOnly, Category = "Combat|Combo")
	float ComboWindowSeconds = 1.2f;

	UPROPERTY(EditDefaultsOnly, Category = "Combat|Damage")
	TSubclassOf<UDamageType> DamageTypeClass;

	bool bIsAttacking = false;
	bool bDamageWindowOpen = false;
	int32 ComboIndex = 0;
	float CurrentDamage = 0.f;
	double LastAttackTime = 0.0;

	/** Actors already hit during the current damage window (avoid multi-hits). */
	UPROPERTY()
	TSet<TObjectPtr<AActor>> HitActorsThisWindow;

	FTimerHandle AttackResetTimer;
	FTimerHandle DamageFallbackTimer;
};
