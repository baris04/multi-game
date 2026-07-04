#include "Enemies/EnemyBase.h"
#include "Enemies/EnemyAIController.h"
#include "Components/HealthComponent.h"
#include "UI/EnemyHealthBarWidget.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	Team = ETeam::Enemies;

	AIControllerClass = AEnemyAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 360.f, 0.f);
		Move->MaxWalkSpeed = 320.f;
	}

	bUseControllerRotationYaw = false;

	// Floating health bar above the enemy (screen-facing, drawn in world space).
	HealthBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidget"));
	HealthBarWidgetComp->SetupAttachment(GetCapsuleComponent());
	HealthBarWidgetComp->SetWidgetClass(UEnemyHealthBarWidget::StaticClass());
	HealthBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
	HealthBarWidgetComp->SetDrawSize(FVector2D(140.f, 18.f));
	HealthBarWidgetComp->SetRelativeLocation(FVector(0.f, 0.f, 110.f));
	HealthBarWidgetComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	HealthBarWidgetComp->SetGenerateOverlapEvents(false);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && !GetController())
	{
		SpawnDefaultController();
	}

	// Bind the floating bar to this enemy's health component. Only local (client/standalone)
	// instances need the widget; on a dedicated server it stays inert.
	if (HealthBarWidgetComp)
	{
		HealthBarWidgetComp->InitWidget();
		if (UEnemyHealthBarWidget* Bar = Cast<UEnemyHealthBarWidget>(HealthBarWidgetComp->GetUserWidgetObject()))
		{
			Bar->SetBarColor(HealthBarColor);
			Bar->SetHealthComponent(GetHealthComponent());
		}
	}
}

void AEnemyBase::PerformAttack(AActor* Target)
{
	// Base does nothing; melee/caster subclasses implement their attack.
}

void AEnemyBase::OnDeathEffects()
{
	Super::OnDeathEffects();

	// Hide the health bar on death.
	if (HealthBarWidgetComp)
	{
		HealthBarWidgetComp->SetVisibility(false);
	}

	// Stop the AI and clean up the corpse after a delay.
	if (AController* C = GetController())
	{
		C->UnPossess();
	}

	SetLifeSpan(CorpseLifeSpan);
}
