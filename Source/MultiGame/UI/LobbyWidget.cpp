#include "UI/LobbyWidget.h"
#include "Core/MultiGameInstance.h"
#include "Core/MultiGamePlayerController.h"

void ULobbyWidget::SetReady(bool bReady)
{
	if (AMultiGamePlayerController* PC = Cast<AMultiGamePlayerController>(GetOwningPlayer()))
	{
		PC->ServerSetReady(bReady);
	}
}

bool ULobbyWidget::IsHost() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		// On a listen server the host controller has authority locally.
		return PC->HasAuthority();
	}
	return false;
}

void ULobbyWidget::StartMatch()
{
	if (!IsHost())
	{
		return;
	}

	if (UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		GI->HostGame(ArenaMapName);
	}
}
