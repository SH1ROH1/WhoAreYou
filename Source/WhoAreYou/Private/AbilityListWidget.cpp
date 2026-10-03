#include "AbilityListWidget.h"
#include "PlayerCharacter.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/TextBlock.h"
#include "GameFramework/InputSettings.h"
#include "Styling/CoreStyle.h"

void UAbilityListWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	}

	if (HeaderText)
	{
		HeaderText->SetText(FText::FromString(TEXT("CONTROLS")));
	}

	RefreshList();
}

void UAbilityListWidget::SetPlayerCharacter(APlayerCharacter* Character)
{
	PlayerCharacter = Character;
	RefreshList();
}

void UAbilityListWidget::RefreshList()
{
	if (!ListBox)
	{
		return;
	}

	ListBox->ClearChildren();

	AddSection(TEXT("CONTROLS"));
	AddEntry(TEXT("Interact"), TEXT("Interact"));
	AddEntry(TEXT("Sprint"), TEXT("Sprint"));
	AddEntry(TEXT("Crouch"), TEXT("Crouch"));
	AddEntry(TEXT("Jump"), TEXT("Jump"));
	AddEntry(TEXT("Restart"), TEXT("Restart"));
	AddEntry(TEXT("Pause"), TEXT("Pause"));

	if (!PlayerCharacter)
	{
		PlayerCharacter = Cast<APlayerCharacter>(GetOwningPlayerPawn());
	}

	if (!PlayerCharacter)
	{
		return;
	}

	AddSection(TEXT("ABILITIES"));

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::Dash))
	{
		AddEntry(TEXT("Dash"), TEXT("Dash"));
	}

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::HighJump))
	{
		AddEntry(TEXT("High Jump"), TEXT("Jump"));
	}

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::DoubleJump))
	{
		AddEntry(TEXT("Double Jump"), TEXT("Jump"));
	}

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::LedgeGrab))
	{
		AddEntry(TEXT("Ledge Grab"), TEXT("Grab"));
	}

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::Vision))
	{
		AddEntry(TEXT("Vision"), TEXT("Vision"));
	}

	if (PlayerCharacter->IsAbilityEnabled(EPlayerAbility::Glide))
	{
		AddEntry(TEXT("Glide"), TEXT("Glide"));
	}

	AddSection(TEXT("CURSES"));

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::Dash))
	{
		AddEntry(TEXT(""), TEXT("Inertia after dash"), false);
	}

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::HighJump))
	{
		AddEntry(TEXT(""), TEXT("Fear of heights"), false);
	}

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::DoubleJump))
	{
		AddEntry(TEXT(""), TEXT("Reduced air control"), false);
	}

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::LedgeGrab))
	{
		AddEntry(TEXT(""), TEXT("Unpredictable ledge grabs"), false);
	}

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::Vision))
	{
		AddEntry(TEXT(""), TEXT("False objects and passages"), false);
	}

	if (PlayerCharacter->IsCurseEnabled(EPlayerCurse::Glide))
	{
		AddEntry(TEXT(""), TEXT("Reduced movement control"), false);
	}
}

void UAbilityListWidget::AddSection(const FString& Title)
{
	UTextBlock* Text = NewObject<UTextBlock>(ListBox);
	Text->SetText(FText::FromString(Title));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor(0.3f, 0.8f, 1.f)));
	Text->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), FontSize + 2));

	UVerticalBoxSlot* BoxSlot = ListBox->AddChildToVerticalBox(Text);

	if (BoxSlot)
	{
		BoxSlot->SetPadding(FMargin(0.f, 12.f, 0.f, 5.f));
	}
}

void UAbilityListWidget::AddEntry(const FString& Name, const FString& ActionName, bool bShowKey)
{
	FString EntryText;

	if (bShowKey)
	{
		const FString Keys = GetActionKeys(FName(*ActionName));

		if (Keys.IsEmpty())
		{
			return;
		}

		EntryText = FString::Printf(TEXT("[%s] %s"), *Keys, *Name);
	}
	else
	{
		EntryText = ActionName;
	}

	UTextBlock* Text = NewObject<UTextBlock>(ListBox);
	Text->SetText(FText::FromString(EntryText));
	Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
	Text->SetFont(FSlateFontInfo(FCoreStyle::GetDefaultFont(), FontSize));

	UVerticalBoxSlot* BoxSlot = ListBox->AddChildToVerticalBox(Text);

	if (BoxSlot)
	{
		BoxSlot->SetPadding(FMargin(0.f, 2.f));
	}
}

FString UAbilityListWidget::GetActionKeys(FName ActionName) const
{
	const UInputSettings* Settings = GetDefault<UInputSettings>();

	if (!Settings)
	{
		return FString();
	}

	TArray<FInputActionKeyMapping> Mappings;
	Settings->GetActionMappingByName(ActionName, Mappings);

	TArray<FString> Keys;

	for (const FInputActionKeyMapping& Mapping : Mappings)
	{
		FString KeyName = Mapping.Key.GetDisplayName().ToString();

		if (Mapping.bShift)
		{
			KeyName = TEXT("Shift + ") + KeyName;
		}

		if (Mapping.bCtrl)
		{
			KeyName = TEXT("Ctrl + ") + KeyName;
		}

		if (Mapping.bAlt)
		{
			KeyName = TEXT("Alt + ") + KeyName;
		}

		if (Mapping.bCmd)
		{
			KeyName = TEXT("Cmd + ") + KeyName;
		}

		Keys.AddUnique(KeyName);
	}

	return FString::Join(Keys, TEXT(" / "));
}