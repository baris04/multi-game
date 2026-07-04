#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Core/MultiGameTypes.h"
#include "MultiGamePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAppearanceUpdated);

/**
 * Replicates a player's chosen appearance and lobby-ready flag to everyone.
 */
UCLASS()
class MULTIGAME_API AMultiGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AMultiGamePlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(BlueprintAssignable, Category = "MultiGame")
	FOnAppearanceUpdated OnAppearanceUpdated;

	UFUNCTION(BlueprintPure, Category = "MultiGame")
	const FCharacterAppearance& GetAppearance() const { return Appearance; }

	/** Server-only setter; replicates to all clients. */
	void SetAppearance(const FCharacterAppearance& NewAppearance);

	UFUNCTION(BlueprintPure, Category = "MultiGame")
	bool IsReady() const { return bIsReady; }

	void SetReady(bool bNewReady);

protected:
	UFUNCTION()
	void OnRep_Appearance();

	UPROPERTY(ReplicatedUsing = OnRep_Appearance)
	FCharacterAppearance Appearance;

	UPROPERTY(Replicated)
	bool bIsReady = false;
};
