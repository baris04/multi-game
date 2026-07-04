#include "Enemies/EnemyAIController.h"
#include "Enemies/EnemyBase.h"
#include "Characters/MultiGameCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EngineUtils.h"

AEnemyAIController::AEnemyAIController()
{
	PrimaryActorTick.bCanEverTick = true;

	PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComp"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = SightRadius;
	SightConfig->LoseSightRadius = LoseSightRadius;
	SightConfig->PeripheralVisionAngleDegrees = 120.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	PerceptionComp->ConfigureSense(*SightConfig);
	PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());
}

void AEnemyAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ControlledEnemy = Cast<AEnemyBase>(InPawn);

	if (PerceptionComp)
	{
		PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(this, &AEnemyAIController::OnTargetPerceptionUpdated);
	}
}

void AEnemyAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
}

AActor* AEnemyAIController::FindBestTarget() const
{
	if (!ControlledEnemy)
	{
		return nullptr;
	}

	// Direct scan: works without perception team setup or nav mesh.
	AMultiGameCharacter* Nearest = nullptr;
	float BestDistSq = FMath::Square(SightRadius);

	for (TActorIterator<AMultiGameCharacter> It(GetWorld()); It; ++It)
	{
		AMultiGameCharacter* Char = *It;
		if (!Char || Char->IsDead() || !ControlledEnemy->IsHostileTo(Char))
		{
			continue;
		}

		const float DistSq = FVector::DistSquared(Char->GetActorLocation(), ControlledEnemy->GetActorLocation());
		if (DistSq < BestDistSq)
		{
			BestDistSq = DistSq;
			Nearest = Char;
		}
	}

	return Nearest;
}

bool AEnemyAIController::HasLineOfSight(AActor* Target) const
{
	return Target != nullptr && LineOfSightTo(Target);
}

void AEnemyAIController::RetreatFrom(AActor* Target)
{
	if (!ControlledEnemy || !Target)
	{
		return;
	}

	const FVector Away = (ControlledEnemy->GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
	ControlledEnemy->AddMovementInput(Away, 1.f);
}

void AEnemyAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!ControlledEnemy || ControlledEnemy->IsDead())
	{
		return;
	}

	CurrentTarget = FindBestTarget();
	if (!CurrentTarget)
	{
		return;
	}

	const float Distance = FVector::Dist(ControlledEnemy->GetActorLocation(), CurrentTarget->GetActorLocation());
	const float Preferred = ControlledEnemy->GetPreferredDistance();
	const float AttackRange = ControlledEnemy->GetAttackRange();

	SetFocus(CurrentTarget);

	// Face the target so attacks and projectiles go the right way.
	const FRotator FaceRot = (CurrentTarget->GetActorLocation() - ControlledEnemy->GetActorLocation()).Rotation();
	ControlledEnemy->SetActorRotation(FRotator(0.f, FaceRot.Yaw, 0.f));

	// Chase the target (works without nav mesh).
	const FVector ToTarget = (CurrentTarget->GetActorLocation() - ControlledEnemy->GetActorLocation()).GetSafeNormal2D();
	ControlledEnemy->AddMovementInput(ToTarget, 1.f);

	if (ControlledEnemy->WantsToKeepDistance() && Distance < Preferred - DistanceTolerance)
	{
		RetreatFrom(CurrentTarget);
	}

	const double Now = FPlatformTime::Seconds();
	if (Distance <= AttackRange && (Now - LastAttackTime) >= ControlledEnemy->GetAttackCooldown())
	{
		LastAttackTime = Now;
		ControlledEnemy->PerformAttack(CurrentTarget);
	}
}
