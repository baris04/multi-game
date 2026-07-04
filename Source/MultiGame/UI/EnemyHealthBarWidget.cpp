#include "UI/EnemyHealthBarWidget.h"
#include "Components/HealthComponent.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/ProgressBar.h"

bool UEnemyHealthBarWidget::Initialize()
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

	// Dark backdrop so the bar reads against any background.
	UProgressBar* Background = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	Background->SetPercent(1.f);
	Background->SetFillColorAndOpacity(FLinearColor(0.05f, 0.05f, 0.05f, 0.85f));
	if (UCanvasPanelSlot* BgSlot = Root->AddChildToCanvas(Background))
	{
		BgSlot->SetPosition(FVector2D(0.f, 0.f));
		BgSlot->SetSize(FVector2D(140.f, 18.f));
	}

	HealthBar = WidgetTree->ConstructWidget<UProgressBar>(UProgressBar::StaticClass());
	HealthBar->SetPercent(1.f);
	HealthBar->SetFillColorAndOpacity(BarColor);
	if (UCanvasPanelSlot* BarSlot = Root->AddChildToCanvas(HealthBar))
	{
		BarSlot->SetPosition(FVector2D(3.f, 3.f));
		BarSlot->SetSize(FVector2D(134.f, 12.f));
	}

	return true;
}

void UEnemyHealthBarWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!HealthBar)
	{
		return;
	}

	if (HealthComponent.IsValid())
	{
		HealthBar->SetPercent(HealthComponent->GetHealthPercent());
	}
}

void UEnemyHealthBarWidget::SetHealthComponent(UHealthComponent* InHealthComponent)
{
	HealthComponent = InHealthComponent;
	if (HealthBar && HealthComponent.IsValid())
	{
		HealthBar->SetPercent(HealthComponent->GetHealthPercent());
	}
}

void UEnemyHealthBarWidget::SetBarColor(const FLinearColor& Color)
{
	BarColor = Color;
	if (HealthBar)
	{
		HealthBar->SetFillColorAndOpacity(BarColor);
	}
}
