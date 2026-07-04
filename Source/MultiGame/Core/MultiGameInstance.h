#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Core/MultiGameTypes.h"
#include "MultiGameInstance.generated.h"

/**
 * Holds cross-map state (character appearance) and drives LAN host/join travel.
 */
UCLASS()
class MULTIGAME_API UMultiGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UMultiGameInstance();

	/** Appearance selected in character creation, carried across level travel. */
	UPROPERTY(BlueprintReadWrite, Category = "MultiGame|Character")
	FCharacterAppearance PendingAppearance;

	/** Host a listen server on the arena map so LAN clients can connect. */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Network")
	void HostGame(const FString& MapName = TEXT("Arena"));

	/** Travel to a lobby map as a listen server (players gather before starting). */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Network")
	void HostLobby();

	/** Connect to a host by IP address (or hostname) over LAN. */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Network")
	void JoinGame(const FString& IpAddress);

	/** Leave the current session and return to the main menu. */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Network")
	void LeaveGame();

	/** Persist current appearance to disk. */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Character")
	void SaveAppearance();

	/** Load appearance from disk into PendingAppearance (if a save exists). */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Character")
	bool LoadAppearance();

private:
	bool bIsTraveling = false;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Config")
	FString MainMenuMapName = TEXT("MainMenu");

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Config")
	FString LobbyMapName = TEXT("Lobby");

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Config")
	FString MapFolder = TEXT("/Game/MultiGame/Maps/");

	static const FString SaveSlotName;
};
