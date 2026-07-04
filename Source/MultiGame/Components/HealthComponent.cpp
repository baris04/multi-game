#include "Components/HealthComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	// Bind on every machine; only the server applies damage in the handler.
	if (AActor* Owner = GetOwner())
	{
		Owner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, bIsDead);
}

void UHealthComponent::ApplyDamageServer(float Damage, AActor* DamageCauser, AController* InstigatedByController)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (Damage <= 0.f || bIsDead || bInvulnerable)
	{
		return;
	}

	if (DamageReceiveCooldown > 0.f)
	{
		const double Now = FPlatformTime::Seconds();
		if ((Now - LastDamageReceiveTime) < DamageReceiveCooldown)
		{
			return;
		}
		LastDamageReceiveTime = Now;
	}

	const float ScaledDamage = Damage * IncomingDamageMultiplier;
	if (ScaledDamage <= 0.f)
	{
		return;
	}

	Health = FMath::Clamp(Health - ScaledDamage, 0.f, MaxHealth);

	AActor* InstigatorActor = InstigatedByController ? InstigatedByController->GetPawn() : DamageCauser;
	OnHealthChanged.Broadcast(this, Health, MaxHealth, InstigatorActor);

	if (Health <= 0.f)
	{
		bIsDead = true;
		OnDeath.Broadcast(GetOwner(), InstigatorActor);
	}
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser)
{
	ApplyDamageServer(Damage, DamageCauser, InstigatedBy);
}

void UHealthComponent::Heal(float Amount)
{
	if (Amount <= 0.f || bIsDead || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	Health = FMath::Clamp(Health + Amount, 0.f, MaxHealth);
	OnHealthChanged.Broadcast(this, Health, MaxHealth, nullptr);
}

void UHealthComponent::InitMaxHealth(float InMaxHealth)
{
	if (InMaxHealth <= 0.f)
	{
		return;
	}

	MaxHealth = InMaxHealth;

	if (GetOwner() && GetOwner()->HasAuthority())
	{
		Health = InMaxHealth;
		bIsDead = false;
	}
}

void UHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, MaxHealth, nullptr);

	if (Health <= 0.f && !bIsDead)
	{
		bIsDead = true;
	}

	if (bIsDead)
	{
		OnDeath.Broadcast(GetOwner(), nullptr);
	}
}
