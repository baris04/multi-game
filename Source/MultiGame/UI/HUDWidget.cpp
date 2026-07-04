#include "UI/HUDWidget.h"
#include "UI/UIFactory.h"
#include "Components/HealthComponent.h"
#include "Enemies/BossCharacter.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/VerticalBox.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

using namespace MultiGameUI;

bool UHUDWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}
	if (!WidgetTree)
	{
		return true;
	}

	UCanvasPanel* Root = WidgetTree->ConstructWidget<UCanvasPanel>(UCanvasPanel::StaticClass(), TEXT("Root"));
	WidgetTree->RootWidget = Root;

	// --- Bottom-left: player health + wave ---
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());

		WaveText = MakeText(WidgetTree, TEXT("Wave 0"), 22);
		Box->AddChildToVerticalBox(WaveText);

		HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		HealthBar->SetFillColorAndOpacity(FLinearColor(0.1f, 0.9f, 0.2f));
		HealthBar->SetPercent(1.f);
		UVerticalBoxSlot* HBSlot = Box->AddChildToVerticalBox(HealthBar);
		HBSlot->SetPadding(FMargin(0.f, 4.f));

		UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(Box);
		CanvasSlot->SetPosition(FVector2D(40.f, -110.f));
		CanvasSlot->SetSize(FVector2D(320.f, 80.f));
		CanvasSlot->SetAnchors(FAnchors(0.f, 1.f)); // bottom-left
		CanvasSlot->SetAlignment(FVector2D(0.f, 1.f));
	}

	// --- Top-center: boss health (hidden until boss active) ---
	{
		UVerticalBox* Box = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass());
		BossContainer = Box;

		UTextBlock* BossLabel = MakeText(WidgetTree, TEXT("BOSS"), 20, FLinearColor(1.f, 0.5f, 0.f));
		BossLabel->SetJustification(ETextJustify::Center);
		Box->AddChildToVerticalBox(BossLabel);

		BossBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
		BossBar->SetFillColorAndOpacity(FLinearColor(0.9f, 0.2f, 0.1f));
		BossBar->SetPercent(1.f);
		Box->AddChildToVerticalBox(BossBar);

		UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(Box);
		CanvasSlot->SetPosition(FVector2D(0.f, 40.f));
		CanvasSlot->SetSize(FVector2D(700.f, 60.f));
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.f));

		Box->SetVisibility(ESlateVisibility::Hidden);
	}

	// --- Center: status (win/lose) ---
	{
		StatusText = MakeText(WidgetTree, TEXT(""), 56, FLinearColor::Yellow);
		StatusText->SetJustification(ETextJustify::Center);
		UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(StatusText);
		CanvasSlot->SetPosition(FVector2D(0.f, 0.f));
		CanvasSlot->SetSize(FVector2D(900.f, 120.f));
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.4f));
		CanvasSlot->SetAlignment(FVector2D(0.5f, 0.5f));
	}

	// --- Top-left: controls reminder ---
	{
		HelpText = MakeText(WidgetTree,
			TEXT("WASD move | Mouse look | LMB attack | RMB heavy | Q/E ability | Shift sprint | Ctrl dodge"),
			16, FLinearColor(0.85f, 0.85f, 0.85f));
		UCanvasPanelSlot* CanvasSlot = Root->AddChildToCanvas(HelpText);
		CanvasSlot->SetPosition(FVector2D(40.f, 40.f));
		CanvasSlot->SetSize(FVector2D(900.f, 40.f));
		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
	}

	return true;
}

void UHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (HealthBar)
	{
		HealthBar->SetPercent(GetLocalHealthPercent());
	}

	const bool bBoss = IsBossActive();
	if (BossContainer)
	{
		BossContainer->SetVisibility(bBoss ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden);
	}
	if (bBoss && BossBar)
	{
		BossBar->SetPercent(GetBossHealthPercent());
	}

	if (WaveText)
	{
		WaveText->SetText(FText::FromString(FString::Printf(TEXT("Wave %d"), GetCurrentWave())));
	}

	if (StatusText)
	{
		switch (GetMatchPhase())
		{
		case EMatchPhase::Won:
			StatusText->SetText(FText::FromString(TEXT("VICTORY!")));
			break;
		case EMatchPhase::Lost:
			StatusText->SetText(FText::FromString(TEXT("DEFEATED")));
			break;
		default:
			StatusText->SetText(FText::GetEmpty());
			break;
		}
	}
}

UHealthComponent* UHUDWidget::GetLocalHealthComponent() const
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (APawn* Pawn = PC->GetPawn())
		{
			return Pawn->FindComponentByClass<UHealthComponent>();
		}
	}
	return nullptr;
}

float UHUDWidget::GetLocalHealthPercent() const
{
	if (UHealthComponent* Health = GetLocalHealthComponent())
	{
		return Health->GetHealthPercent();
	}
	return 0.f;
}

float UHUDWidget::GetBossHealthPercent() const
{
	if (AMultiGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMultiGameState>() : nullptr)
	{
		if (ABossCharacter* Boss = GS->GetBoss())
		{
			if (UHealthComponent* Health = Boss->GetHealthComponent())
			{
				return Health->GetHealthPercent();
			}
		}
	}
	return 0.f;
}

bool UHUDWidget::IsBossActive() const
{
	if (AMultiGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMultiGameState>() : nullptr)
	{
		return GS->GetBoss() != nullptr && GS->GetMatchPhase() == EMatchPhase::BossFight;
	}
	return false;
}

int32 UHUDWidget::GetCurrentWave() const
{
	if (AMultiGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMultiGameState>() : nullptr)
	{
		return GS->GetCurrentWave();
	}
	return 0;
}

EMatchPhase UHUDWidget::GetMatchPhase() const
{
	if (AMultiGameState* GS = GetWorld() ? GetWorld()->GetGameState<AMultiGameState>() : nullptr)
	{
		return GS->GetMatchPhase();
	}
	return EMatchPhase::WaitingToStart;
}
