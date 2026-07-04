#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnHealthChanged, UHealthComponent*, HealthComp, float, Health, float, MaxHealth, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeath, AActor*, DeadActor, AActor*, KillerActor);

/**
 * Server-authoritative health/damage container shared by players and enemies.
 */
UCLASS(ClassGroup = (MultiGame), meta = (BlueprintSpawnableComponent))
class MULTIGAME_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Broadcast whenever health changes (both server and clients via OnRep). */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnHealthChanged OnHealthChanged;

	/** Broadcast once when health reaches zero. */
	UPROPERTY(BlueprintAssignable, Category = "Health")
	FOnDeath OnDeath;

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealth() const { return Health; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetMaxHealth() const { return MaxHealth; }

	UFUNCTION(BlueprintPure, Category = "Health")
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	UFUNCTION(BlueprintPure, Category = "Health")
	bool IsDead() const { return bIsDead; }

	/** Server-only: apply healing (clamped to MaxHealth). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void Heal(float Amount);

	/** Server-only: apply damage directly (bypasses broken overlap/damage pipelines). */
	void ApplyDamageServer(float Damage, AActor* DamageCauser, AController* InstigatedByController = nullptr);

	/** Server-only: set max/current health at spawn (e.g. lower HP for grunts). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void InitMaxHealth(float InMaxHealth);

	/** Temporary damage immunity window (e.g. during a dodge roll). */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetInvulnerable(bool bNewInvulnerable) { bInvulnerable = bNewInvulnerable; }

	/** Scales incoming damage (1 = normal). Set lower on player pawns for easier fights. */
	void SetIncomingDamageMultiplier(float Multiplier) { IncomingDamageMultiplier = FMath::Max(0.f, Multiplier); }

	/** Minimum seconds between damage ticks (prevents burst one-shots). 0 = disabled. */
	void SetDamageReceiveCooldown(float Seconds) { DamageReceiveCooldown = FMath::Max(0.f, Seconds); }

protected:
	virtual void BeginPlay() override;

	/** Bound to the owner's OnTakeAnyDamage so damage flows through here. */
	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	float Health = 100.f;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bIsDead = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Health")
	bool bInvulnerable = false;

	float IncomingDamageMultiplier = 1.f;
	float DamageReceiveCooldown = 0.f;
	double LastDamageReceiveTime = -1000.0;
};
