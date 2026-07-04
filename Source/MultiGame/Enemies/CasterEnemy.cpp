#include "Enemies/CasterEnemy.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "Components/AbilityComponent.h"
#include "Components/HealthComponent.h"

ACasterEnemy::ACasterEnemy()
{
	AbilityComponent = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComponent"));

	PreferredDistance = 700.f;
	AttackRange = 1000.f;
	AttackCooldown = 5.f;

	PlaceholderColor = FLinearColor(0.6f, 0.1f, 0.9f);
	HealthBarColor = FLinearColor(0.7f, 0.2f, 1.f);

	MultiGameMannequin::ApplyVisualPreset(this, MultiGameMannequin::EVisualPreset::CasterEnemy);
}

void ACasterEnemy::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		HealthComponent->InitMaxHealth(110.f);
	}

	if (AbilityComponent)
	{
		AbilityComponent->ConfigureAbility(0, 6.f, 4.f);
	}
}

void ACasterEnemy::PerformAttack(AActor* Target)
{
	if (AbilityComponent && Target)
	{
		AbilityComponent->TryCastAbility(0, Target->GetActorLocation());
	}
}
