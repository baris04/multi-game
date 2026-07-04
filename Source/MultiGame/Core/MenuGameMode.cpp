#include "Core/MenuGameMode.h"
#include "Core/MultiGameSceneSetup.h"
#include "UI/MainMenuWidget.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "Kismet/GameplayStatics.h"

AMenuGameMode::AMenuGameMode()
{
	MenuWidgetClass = UMainMenuWidget::StaticClass();

	// Menu has no gameplay pawn; players stay as spectators so spawn never fails.
	bStartPlayersAsSpectators = true;
	DefaultPawnClass = nullptr;
}

void AMenuGameMode::BeginPlay()
{
	Super::BeginPlay();

	MultiGameSceneSetup::ApplyGameplayLighting(GetWorld());

	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> SkyLights;
		UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
		for (AActor* Actor : SkyLights)
		{
			if (ASkyLight* Sky = Cast<ASkyLight>(Actor))
			{
				if (USkyLightComponent* Light = Sky->GetLightComponent())
				{
					Light->RecaptureSky();
				}
			}
		}
	}

	APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr;
	if (!PC || !MenuWidgetClass)
	{
		return;
	}

	MenuWidget = CreateWidget<UUserWidget>(PC, MenuWidgetClass);
	if (MenuWidget)
	{
		MenuWidget->AddToViewport();
	}

	// Menu needs a cursor and UI input.
	PC->bShowMouseCursor = true;
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PC->SetInputMode(InputMode);
}
