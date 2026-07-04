#include "Enemies/MeleeEnemy.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "Components/CombatComponent.h"
#include "Components/HealthComponent.h"

AMeleeEnemy::AMeleeEnemy()
{
	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetDamageValues(4.f, 7.f);

	PreferredDistance = 120.f;
	AttackRange = 200.f;
	AttackCooldown = 3.5f;

	PlaceholderColor = FLinearColor(0.9f, 0.15f, 0.1f);

	MultiGameMannequin::ApplyVisualPreset(this, MultiGameMannequin::EVisualPreset::MeleeEnemy);
}

void AMeleeEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		HealthComponent->InitMaxHealth(160.f);
	}

	// Tint the Gideon model red so melee enemies read clearly as hostiles.
	ApplyCharacterTint(FLinearColor(1.f, 0.25f, 0.2f));
}

void AMeleeEnemy::PerformAttack(AActor* Target)
{
	if (CombatComponent)
	{
		CombatComponent->RequestLightAttack();
	}
}
