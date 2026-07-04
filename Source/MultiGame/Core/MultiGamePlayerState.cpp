#include "Core/MultiGamePlayerState.h"
#include "Characters/PlayerCharacter.h"
#include "Net/UnrealNetwork.h"

AMultiGamePlayerState::AMultiGamePlayerState()
{
}

void AMultiGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMultiGamePlayerState, Appearance);
	DOREPLIFETIME(AMultiGamePlayerState, bIsReady);
}

void AMultiGamePlayerState::SetAppearance(const FCharacterAppearance& NewAppearance)
{
	if (!HasAuthority())
	{
		return;
	}

	Appearance = NewAppearance;

	// Apply immediately on the server; clients apply through OnRep.
	OnRep_Appearance();
}

void AMultiGamePlayerState::SetReady(bool bNewReady)
{
	if (HasAuthority())
	{
		bIsReady = bNewReady;
	}
}

void AMultiGamePlayerState::OnRep_Appearance()
{
	OnAppearanceUpdated.Broadcast();

	if (APlayerCharacter* PC = Cast<APlayerCharacter>(GetPawn()))
	{
		PC->ApplyAppearance(Appearance);
	}
}
