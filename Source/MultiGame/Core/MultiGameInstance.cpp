#include "Core/MultiGameInstance.h"
#include "Save/PlayerCharacterSaveGame.h"
#include "Kismet/GameplayStatics.h"

const FString UMultiGameInstance::SaveSlotName = TEXT("PlayerCharacter");

UMultiGameInstance::UMultiGameInstance()
{
}

void UMultiGameInstance::HostGame(const FString& MapName)
{
	if (bIsTraveling)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		if (APlayerController* PC = GetFirstLocalPlayerController())
		{
			World = PC->GetWorld();
		}
	}
	if (!World)
	{
		return;
	}

	bIsTraveling = true;

	const FString MapPath = FString::Printf(TEXT("%s%s"), *MapFolder, *MapName);
	// Additional URL options must use '&', not a second '?'.
	const FString Options = TEXT("listen&game=/Script/MultiGame.MultiGameMode");

	// Standalone PIE (not listen server): OpenLevel works more reliably than ServerTravel.
	if (World->GetNetMode() == NM_Standalone)
	{
		UGameplayStatics::OpenLevel(this, FName(*MapPath), true, Options);
		return;
	}

	const FString TravelURL = FString::Printf(TEXT("%s?%s"), *MapPath, *Options);
	World->ServerTravel(TravelURL, true);
}

void UMultiGameInstance::HostLobby()
{
	HostGame(LobbyMapName);
}

void UMultiGameInstance::JoinGame(const FString& IpAddress)
{
	APlayerController* PC = GetFirstLocalPlayerController();
	if (!PC || IpAddress.IsEmpty())
	{
		return;
	}

	PC->ClientTravel(IpAddress, TRAVEL_Absolute);
}

void UMultiGameInstance::LeaveGame()
{
	if (APlayerController* PC = GetFirstLocalPlayerController())
	{
		const FString Travel = FString::Printf(TEXT("%s%s"), *MapFolder, *MainMenuMapName);
		PC->ClientTravel(Travel, TRAVEL_Absolute);
	}
}

void UMultiGameInstance::SaveAppearance()
{
	UPlayerCharacterSaveGame* Save = Cast<UPlayerCharacterSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPlayerCharacterSaveGame::StaticClass()));
	if (!Save)
	{
		return;
	}

	Save->Appearance = PendingAppearance;
	Save->SlotName = SaveSlotName;
	UGameplayStatics::SaveGameToSlot(Save, SaveSlotName, 0);
}

bool UMultiGameInstance::LoadAppearance()
{
	if (!UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0))
	{
		return false;
	}

	if (UPlayerCharacterSaveGame* Save = Cast<UPlayerCharacterSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SaveSlotName, 0)))
	{
		PendingAppearance = Save->Appearance;
		return true;
	}

	return false;
}
