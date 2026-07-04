#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LobbyWidget.generated.h"

/**
 * Base class for WBP_Lobby. Lists connected players (built in Blueprint from the
 * PlayerArray), toggles ready state, and lets the host start the match.
 */
UCLASS(Abstract)
class MULTIGAME_API ULobbyWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void SetReady(bool bReady);

	/** Host-only: travel everyone to the arena to begin. */
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void StartMatch();

	UFUNCTION(BlueprintPure, Category = "Lobby")
	bool IsHost() const;

	/** Rebuild the player list; implement in Blueprint. */
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby")
	void RefreshPlayerList();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Lobby")
	FString ArenaMapName = TEXT("Arena");
};
