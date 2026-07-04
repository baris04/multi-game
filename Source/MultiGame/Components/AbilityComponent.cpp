#include "Components/AbilityComponent.h"
#include "Characters/MultiGameCharacter.h"
#include "Combat/ProjectileBase.h"
#include "Combat/CombatVisuals.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UAbilityComponent::UAbilityComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	FAbilityDefinition Bolt;
	Bolt.AbilityId = TEXT("Fireball");
	Bolt.ProjectileClass = AProjectileBase::StaticClass();
	Bolt.Damage = 35.f;
	Bolt.Cooldown = 1.2f;
	Bolt.CastTime = 0.f;
	Abilities.Add(Bolt);

	FAbilityDefinition Bolt2 = Bolt;
	Bolt2.AbilityId = TEXT("Fireball2");
	Abilities.Add(Bolt2);
}

void UAbilityComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AMultiGameCharacter>(GetOwner());
	EnsureDefaultProjectileAbility();
	CooldownEndTimes.Init(0.0, Abilities.Num());
}

void UAbilityComponent::EnsureDefaultProjectileAbility()
{
	if (Abilities.Num() == 0)
	{
		FAbilityDefinition Bolt;
		Bolt.AbilityId = TEXT("Fireball");
		Bolt.ProjectileClass = AProjectileBase::StaticClass();
		Bolt.Damage = 18.f;
		Bolt.Cooldown = 1.5f;
		Bolt.CastTime = 0.f;
		Abilities.Add(Bolt);
	}

	for (FAbilityDefinition& Ability : Abilities)
	{
		if (!Ability.ProjectileClass)
		{
			Ability.ProjectileClass = AProjectileBase::StaticClass();
		}
	}
}

void UAbilityComponent::ConfigureAbility(int32 AbilityIndex, float Damage, float Cooldown)
{
	EnsureDefaultProjectileAbility();
	if (Abilities.IsValidIndex(AbilityIndex))
	{
		Abilities[AbilityIndex].Damage = Damage;
		Abilities[AbilityIndex].Cooldown = Cooldown;
	}
}

bool UAbilityComponent::IsOnCooldown(int32 AbilityIndex) const
{
	if (!CooldownEndTimes.IsValidIndex(AbilityIndex))
	{
		return false;
	}
	return FPlatformTime::Seconds() < CooldownEndTimes[AbilityIndex];
}

float UAbilityComponent::GetCooldownRemaining(int32 AbilityIndex) const
{
	if (!CooldownEndTimes.IsValidIndex(AbilityIndex))
	{
		return 0.f;
	}
	return FMath::Max(0.0, CooldownEndTimes[AbilityIndex] - FPlatformTime::Seconds());
}

bool UAbilityComponent::TryCastAbility(int32 AbilityIndex, FVector TargetLocation)
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	if (!OwnerCharacter || OwnerCharacter->IsDead())
	{
		return false;
	}

	EnsureDefaultProjectileAbility();

	if (!Abilities.IsValidIndex(AbilityIndex) || IsOnCooldown(AbilityIndex))
	{
		return false;
	}

	const FAbilityDefinition& Ability = Abilities[AbilityIndex];

	if (Ability.CastMontage)
	{
		OwnerCharacter->MulticastPlayMontage(Ability.CastMontage, NAME_None);
	}

	if (CooldownEndTimes.IsValidIndex(AbilityIndex))
	{
		CooldownEndTimes[AbilityIndex] = FPlatformTime::Seconds() + Ability.Cooldown;
		OnAbilityCooldownChanged.Broadcast(AbilityIndex, Ability.Cooldown);
	}

	FinishCast(AbilityIndex, TargetLocation);

	return true;
}

void UAbilityComponent::FinishCast(int32 AbilityIndex, FVector TargetLocation)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !OwnerCharacter || !Abilities.IsValidIndex(AbilityIndex))
	{
		return;
	}

	EnsureDefaultProjectileAbility();

	const FAbilityDefinition& Ability = Abilities[AbilityIndex];
	TSubclassOf<AProjectileBase> ProjectileClass = Ability.ProjectileClass;
	if (!ProjectileClass)
	{
		ProjectileClass = AProjectileBase::StaticClass();
	}

	const FVector Direction = OwnerCharacter->GetAbilityAimDirection(TargetLocation);
	const FVector SpawnLocation = OwnerCharacter->GetAbilityMuzzleLocation() + Direction * 60.f;
	const FTransform SpawnTransform(Direction.Rotation(), SpawnLocation);

	AProjectileBase* Projectile = GetWorld()->SpawnActorDeferred<AProjectileBase>(
		ProjectileClass, SpawnTransform, OwnerCharacter, OwnerCharacter,
		ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

	if (!Projectile)
	{
		return;
	}

	Projectile->InitProjectile(Ability.Damage, OwnerCharacter, Direction);
	UGameplayStatics::FinishSpawningActor(Projectile, SpawnTransform);

	OwnerCharacter->MulticastSpawnProjectileVisual(SpawnLocation, Direction, Ability.Damage);

	FCombatVisuals::SpawnGlowSphere(GetWorld(), SpawnLocation, 28.f, FLinearColor(1.f, 0.55f, 0.1f), 0.15f);
}
