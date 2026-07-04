#include "UI/MainMenuWidget.h"
#include "UI/UIFactory.h"
#include "Core/MultiGameInstance.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/VerticalBox.h"
#include "Components/EditableTextBox.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Kismet/KismetSystemLibrary.h"

using namespace MultiGameUI;

bool UMainMenuWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (!WidgetTree)
	{
		return true;
	}

	// Root overlay centers the menu column on screen.
	UOverlay* Root = WidgetTree->ConstructWidget<UOverlay>(UOverlay::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
	UOverlaySlot* BoxSlot = Root->AddChildToOverlay(Box);
	BoxSlot->SetHorizontalAlignment(HAlign_Center);
	BoxSlot->SetVerticalAlignment(VAlign_Center);

	AddToBox(Box, MakeText(WidgetTree, TEXT("MULTI GAME"), 48));
	AddToBox(Box, MakeText(WidgetTree, TEXT("LAN Co-op"), 20, FLinearColor(0.7f, 0.7f, 0.7f)));

	IpTextBox = WidgetTree->ConstructWidget<UEditableTextBox>(UEditableTextBox::StaticClass());
	IpTextBox->SetHintText(FText::FromString(TEXT("Host IP (e.g. 127.0.0.1)")));
	IpTextBox->SetText(FText::FromString(TEXT("127.0.0.1")));
	AddToBox(Box, IpTextBox);

	UButton* HostBtn = MakeButton(WidgetTree, TEXT("HOST GAME"));
	HostButton = HostBtn;
	HostBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnHostClicked);
	AddToBox(Box, HostBtn);

	UButton* JoinBtn = MakeButton(WidgetTree, TEXT("JOIN GAME"));
	JoinButton = JoinBtn;
	JoinBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnJoinClicked);
	AddToBox(Box, JoinBtn);

	UButton* QuitBtn = MakeButton(WidgetTree, TEXT("QUIT"));
	QuitBtn->OnClicked.AddDynamic(this, &UMainMenuWidget::OnQuitClicked);
	AddToBox(Box, QuitBtn);

	return true;
}

void UMainMenuWidget::OnHostClicked()
{
	if (bHostClicked)
	{
		return;
	}
	bHostClicked = true;

	if (HostButton)
	{
		HostButton->SetIsEnabled(false);
	}
	if (JoinButton)
	{
		JoinButton->SetIsEnabled(false);
	}

	if (UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		GI->HostGame(TEXT("Arena"));
	}
}

void UMainMenuWidget::OnJoinClicked()
{
	if (UMultiGameInstance* GI = Cast<UMultiGameInstance>(GetGameInstance()))
	{
		const FString Ip = IpTextBox ? IpTextBox->GetText().ToString() : TEXT("127.0.0.1");
		GI->JoinGame(Ip);
	}
}

void UMainMenuWidget::OnQuitClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
	}
}
