#pragma once

#include "CoreMinimal.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/EditableTextBox.h"
#include "Components/ProgressBar.h"

/**
 * Small helpers to build UMG widgets in C++ so the game needs no WBP assets.
 */
namespace MultiGameUI
{
	inline UTextBlock* MakeText(UWidgetTree* Tree, const FString& Text, int32 FontSize = 20, FLinearColor Color = FLinearColor::White)
	{
		UTextBlock* Block = Tree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass());
		Block->SetText(FText::FromString(Text));
		Block->SetColorAndOpacity(FSlateColor(Color));
		FSlateFontInfo Font = Block->GetFont();
		Font.Size = FontSize;
		Block->SetFont(Font);
		return Block;
	}

	inline UButton* MakeButton(UWidgetTree* Tree, const FString& Label, int32 FontSize = 22)
	{
		UButton* Button = Tree->ConstructWidget<UButton>(UButton::StaticClass());
		UTextBlock* Text = MakeText(Tree, Label, FontSize, FLinearColor::Black);
		Text->SetJustification(ETextJustify::Center);
		Button->AddChild(Text);
		return Button;
	}

	inline UVerticalBoxSlot* AddToBox(UVerticalBox* Box, UWidget* Child, float Padding = 8.f)
	{
		UVerticalBoxSlot* Slot = Box->AddChildToVerticalBox(Child);
		Slot->SetPadding(FMargin(Padding));
		Slot->SetHorizontalAlignment(HAlign_Fill);
		return Slot;
	}
}
