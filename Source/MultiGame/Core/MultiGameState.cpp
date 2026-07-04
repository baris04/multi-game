#include "Core/MultiGameState.h"
#include "Enemies/BossCharacter.h"
#include "Net/UnrealNetwork.h"

AMultiGameState::AMultiGameState()
{
}

void AMultiGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiGameState, MatchPhase);
	DOREPLIFETIME(AMultiGameState, CurrentWave);
	DOREPLIFETIME(AMultiGameState, Boss);
}

void AMultiGameState::SetMatchPhase(EMatchPhase NewPhase)
{
	if (!HasAuthority())
	{
		return;
	}

	MatchPhase = NewPhase;
	OnRep_MatchPhase();
}

void AMultiGameState::SetCurrentWave(int32 NewWave)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentWave = NewWave;
	OnRep_CurrentWave();
}

void AMultiGameState::SetBoss(ABossCharacter* InBoss)
{
	if (HasAuthority())
	{
		Boss = InBoss;
	}
}

void AMultiGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void AMultiGameState::OnRep_CurrentWave()
{
	OnWaveChanged.Broadcast(CurrentWave);
}
