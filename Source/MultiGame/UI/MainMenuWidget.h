#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UEditableTextBox;

/**
 * Main menu built entirely in C++ (no WBP asset needed): Host / Join(IP) / Quit.
 */
UCLASS()
class MULTIGAME_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	UFUNCTION()
	void OnHostClicked();

	UFUNCTION()
	void OnJoinClicked();

	UFUNCTION()
	void OnQuitClicked();

protected:
	UPROPERTY()
	TObjectPtr<UEditableTextBox> IpTextBox;

	UPROPERTY()
	TObjectPtr<class UButton> HostButton;

	UPROPERTY()
	TObjectPtr<class UButton> JoinButton;

	bool bHostClicked = false;
};
