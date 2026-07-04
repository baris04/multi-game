#include "Core/MultiGamePlayerController.h"
#include "Core/MultiGamePlayerState.h"
#include "Core/MultiGameInstance.h"
#include "UI/HUDWidget.h"
#include "Blueprint/UserWidget.h"

AMultiGamePlayerController::AMultiGamePlayerController()
{
	HUDWidgetClass = UHUDWidget::StaticClass();
}

void AMultiGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalController())
	{
		SendAppearanceToServer();

		if (!IsRunningDedicatedServer())
		{
			ShowGameplayHUD();
		}
	}
}

void AMultiGamePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	if (IsLocalController())
	{
		SendAppearanceToServer();
		ShowGameplayHUD();
	}
}

void AMultiGamePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalController())
	{
		ShowGameplayHUD();
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void AMultiGamePlayerController::SendAppearanceToServer()
{
	if (bAppearanceSent)
	{
		return;
	}

	if (UMultiGameInstance* GI = GetGameInstance<UMultiGameInstance>())
	{
		ServerSetAppearance(GI->PendingAppearance);
		bAppearanceSent = true;
	}
}

void AMultiGamePlayerController::ServerSetAppearance_Implementation(const FCharacterAppearance& NewAppearance)
{
	if (AMultiGamePlayerState* PS = GetPlayerState<AMultiGamePlayerState>())
	{
		PS->SetAppearance(NewAppearance);
	}
}

void AMultiGamePlayerController::ServerSetReady_Implementation(bool bReady)
{
	if (AMultiGamePlayerState* PS = GetPlayerState<AMultiGamePlayerState>())
	{
		PS->SetReady(bReady);
	}
}

void AMultiGamePlayerController::ShowGameplayHUD()
{
	if (bHUDShown || !IsLocalController() || !HUDWidgetClass)
	{
		return;
	}

	HUDWidget = CreateWidget<UUserWidget>(this, HUDWidgetClass);
	if (HUDWidget)
	{
		HUDWidget->AddToViewport();
		bHUDShown = true;
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}
