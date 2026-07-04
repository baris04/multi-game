#include "UI/CharacterCreationWidget.h"
#include "Core/MultiGameInstance.h"

void UCharacterCreationWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Seed the editor with any previously saved appearance.
	if (UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		GI->LoadAppearance();
		WorkingAppearance = GI->PendingAppearance;
		OnAppearancePreviewUpdated();
	}
}

void UCharacterCreationWidget::ConfirmAppearance()
{
	if (UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		GI->PendingAppearance = WorkingAppearance;
		GI->SaveAppearance();
	}
}
