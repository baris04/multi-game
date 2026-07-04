#include "Enemies/BossCharacter.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "Components/CombatComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/HealthComponent.h"
#include "Combat/ProjectileBase.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"

ABossCharacter::ABossCharacter()
{
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	AbilityComponent = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComponent"));
	CombatComponent->SetDamageValues(6.f, 10.f);

	// Boss engages at mid-range so it can both melee and cast.
	PreferredDistance = 300.f;
	AttackRange = 1200.f;
	AttackCooldown = 4.f;
	CorpseLifeSpan = 15.f;

	PlaceholderColor = FLinearColor(1.f, 0.55f, 0.f);
	HealthBarColor = FLinearColor(1.f, 0.55f, 0.f);
	GetCapsuleComponent()->SetCapsuleSize(60.f, 140.f);

	MultiGameMannequin::ApplyVisualPreset(this, MultiGameMannequin::EVisualPreset::Boss);
}

void ABossCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		HealthComponent->InitMaxHealth(350.f);
	}

	if (HealthComponent)
	{
		HealthComponent->OnHealthChanged.AddDynamic(this, &ABossCharacter::HandleHealthChanged);
	}

	EnterPhase(1);
}

void ABossCharacter::HandleHealthChanged(UHealthComponent* HealthComp, float Health, float MaxHealth, AActor* InstigatorActor)
{
	if (!HasAuthority() || MaxHealth <= 0.f)
	{
		return;
	}

	const float Percent = Health / MaxHealth;

	// Advance through thresholds (phase = number of thresholds crossed + 1).
	int32 TargetPhase = 1;
	for (const float Threshold : PhaseHealthThresholds)
	{
		if (Percent <= Threshold)
		{
			++TargetPhase;
		}
	}

	if (TargetPhase != CurrentPhase)
	{
		EnterPhase(TargetPhase);
	}
}

void ABossCharacter::EnterPhase(int32 NewPhase)
{
	CurrentPhase = NewPhase;

	const int32 Index = FMath::Clamp(NewPhase - 1, 0, PhaseAttackCooldowns.Num() - 1);
	if (PhaseAttackCooldowns.IsValidIndex(Index))
	{
		AttackCooldown = PhaseAttackCooldowns[Index];
	}

	OnBossPhaseChanged.Broadcast(NewPhase);
}

void ABossCharacter::PerformAttack(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	const float Distance = FVector::Dist(GetActorLocation(), Target->GetActorLocation());
	const int32 PhaseIdx = FMath::Clamp(CurrentPhase - 1, 0, PhaseWaveProjectiles.Num() - 1);
	const int32 WaveCount = PhaseWaveProjectiles.IsValidIndex(PhaseIdx) ? PhaseWaveProjectiles[PhaseIdx] : 0;

	// Close range: alternate melee combo and projectile toward target.
	if (Distance <= 350.f)
	{
		if (bMeleeToggle && CombatComponent)
		{
			CombatComponent->RequestHeavyAttack();
		}
		else if (CombatComponent)
		{
			CombatComponent->RequestLightAttack();
		}
		bMeleeToggle = !bMeleeToggle;
		return;
	}

	// Mid/long range: in later phases fire a wave, otherwise a single bolt.
	if (WaveCount > 0)
	{
		CastProjectileWave(WaveCount);
	}
	else if (AbilityComponent)
	{
		AbilityComponent->TryCastAbility(0, Target->GetActorLocation());
	}
}

void ABossCharacter::CastProjectileWave(int32 ProjectileCount)
{
	if (!HasAuthority() || !WaveProjectileClass || ProjectileCount <= 0)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float AngleStep = 360.f / ProjectileCount;

	for (int32 i = 0; i < ProjectileCount; ++i)
	{
		const float Yaw = AngleStep * i;
		const FRotator SpawnRot(0.f, Yaw, 0.f);
		const FVector SpawnLoc = Origin + SpawnRot.Vector() * 120.f;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (AProjectileBase* Proj = World->SpawnActor<AProjectileBase>(WaveProjectileClass, SpawnLoc, SpawnRot, SpawnParams))
		{
			Proj->InitProjectile(WaveProjectileDamage, this, SpawnRot.Vector());
		}
	}
}
