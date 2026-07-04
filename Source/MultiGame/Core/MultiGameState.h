#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "MultiGameState.generated.h"

UENUM(BlueprintType)
enum class EMatchPhase : uint8
{
	WaitingToStart	UMETA(DisplayName = "WaitingToStart"),
	InProgress		UMETA(DisplayName = "InProgress"),
	BossFight		UMETA(DisplayName = "BossFight"),
	Won				UMETA(DisplayName = "Won"),
	Lost			UMETA(DisplayName = "Lost")
};

class ABossCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChanged, int32, NewWave);

/**
 * Replicates encounter state (match phase, current wave, boss reference) so all
 * clients can drive HUD/objective UI.
 */
UCLASS()
class MULTIGAME_API AMultiGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AMultiGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "MultiGame")
	FOnMatchPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "MultiGame")
	FOnWaveChanged OnWaveChanged;

	UFUNCTION(BlueprintPure, Category = "MultiGame")
	EMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintPure, Category = "MultiGame")
	int32 GetCurrentWave() const { return CurrentWave; }

	UFUNCTION(BlueprintPure, Category = "MultiGame")
	ABossCharacter* GetBoss() const { return Boss; }

	// Server-only setters.
	void SetMatchPhase(EMatchPhase NewPhase);
	void SetCurrentWave(int32 NewWave);
	void SetBoss(ABossCharacter* InBoss);

protected:
	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_CurrentWave();

	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase)
	EMatchPhase MatchPhase = EMatchPhase::WaitingToStart;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWave)
	int32 CurrentWave = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "MultiGame")
	TObjectPtr<ABossCharacter> Boss;
};
