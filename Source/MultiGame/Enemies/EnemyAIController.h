#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionTypes.h"
#include "EnemyAIController.generated.h"

class UAIPerceptionComponent;
class UAISenseConfig_Sight;
class AEnemyBase;

/**
 * Server-only AI: senses hostile players via sight, positions the enemy at its
 * preferred distance, and triggers attacks when in range and off cooldown.
 *
 * NOTE: This is a self-contained C++ state machine so the game is playable without
 * Behavior Tree assets. A Behavior Tree can be layered on top later if desired.
 */
UCLASS()
class MULTIGAME_API AEnemyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyAIController();

	virtual void OnPossess(APawn* InPawn) override;
	virtual void Tick(float DeltaSeconds) override;

protected:
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	AActor* FindBestTarget() const;
	bool HasLineOfSight(AActor* Target) const;
	void RetreatFrom(AActor* Target);

	UPROPERTY(VisibleAnywhere, Category = "AI")
	TObjectPtr<UAIPerceptionComponent> PerceptionComp;

	UPROPERTY()
	TObjectPtr<UAISenseConfig_Sight> SightConfig;

	UPROPERTY()
	TObjectPtr<AEnemyBase> ControlledEnemy;

	UPROPERTY()
	TObjectPtr<AActor> CurrentTarget;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float SightRadius = 1500.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float LoseSightRadius = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	float DistanceTolerance = 75.f;

	double LastAttackTime = 0.0;
};
