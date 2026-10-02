#include "Dialogues/DialogueWidget.h"
#include "Dialogues/DialogueManager.h"
#include "Dialogues/DialogueDataAsset.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "TimerManager.h"

void UDialogueWidget::NativeConstruct()
{
	Super::NativeConstruct();

	NextButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnNextClicked);
	ChoiceAButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnChoiceAClicked);
	ChoiceBButton->OnClicked.AddDynamic(this, &UDialogueWidget::OnChoiceBClicked);

	ChoiceAButton->SetVisibility(ESlateVisibility::Collapsed);
	ChoiceBButton->SetVisibility(ESlateVisibility::Collapsed);
}

void UDialogueWidget::NativeDestruct()
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(TypewriterTimer);
	}

	if (NextButton)
	{
		NextButton->OnClicked.RemoveDynamic(this, &UDialogueWidget::OnNextClicked);
	}

	if (ChoiceAButton)
	{
		ChoiceAButton->OnClicked.RemoveDynamic(this, &UDialogueWidget::OnChoiceAClicked);
	}

	if (ChoiceBButton)
	{
		ChoiceBButton->OnClicked.RemoveDynamic(this, &UDialogueWidget::OnChoiceBClicked);
	}

	Super::NativeDestruct();
}

void UDialogueWidget::ShowNode(const FDialogueNode& Node)
{
	if (!SpeakerText || !DialogueText)
	{
		return;
	}

	SpeakerText->SetText(Node.Speaker);

	bCurrentNodeIsChoice = Node.Type == EDialogueNodeType::Choice;

	ChoiceAText->SetText(Node.ChoiceA);
	ChoiceBText->SetText(Node.ChoiceB);

	SetChoicesVisible(false);

	StartTypewriter(Node.Text);
}

void UDialogueWidget::StartTypewriter(const FText& Text)
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TypewriterTimer);

	FullText = Text.ToString();
	CurrentCharacter = 0;
	bIsTyping = true;

	DialogueText->SetText(FText::GetEmpty());

	constexpr float CharactersPerSecond = 50.0f;
	constexpr float Interval = 1.0f / CharactersPerSecond;

	GetWorld()->GetTimerManager().SetTimer(
		TypewriterTimer,
		this,
		&UDialogueWidget::TypewriterTick,
		Interval,
		true
	);
}

void UDialogueWidget::TypewriterTick()
{
	if (!DialogueText)
	{
		return;
	}

	CurrentCharacter++;

	DialogueText->SetText(FText::FromString(FullText.Left(CurrentCharacter)));

	if (CurrentCharacter >= FullText.Len())
	{
		FinishTypewriter();
	}
}

void UDialogueWidget::FinishTypewriter()
{
	if (!GetWorld())
	{
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(TypewriterTimer);

	CurrentCharacter = FullText.Len();
	bIsTyping = false;

	DialogueText->SetText(FText::FromString(FullText));

	SetChoicesVisible(bCurrentNodeIsChoice);
}

void UDialogueWidget::OnNextClicked()
{
	if (bIsTyping)
	{
		FinishTypewriter();
		return;
	}

	if (bCurrentNodeIsChoice)
	{
		return;
	}

	if (ADialogueManager* Manager = ADialogueManager::Get(this))
	{
		Manager->Next();
	}
}

void UDialogueWidget::OnChoiceAClicked()
{
	if (bIsTyping)
	{
		return;
	}

	if (ADialogueManager* Manager = ADialogueManager::Get(this))
	{
		Manager->SelectChoice(false);
	}
}

void UDialogueWidget::OnChoiceBClicked()
{
	if (bIsTyping)
	{
		return;
	}

	if (ADialogueManager* Manager = ADialogueManager::Get(this))
	{
		Manager->SelectChoice(true);
	}
}

void UDialogueWidget::SetChoicesVisible(bool bVisible)
{
	const ESlateVisibility ChoiceVisibility = bVisible
		? ESlateVisibility::Visible
		: ESlateVisibility::Collapsed;

	ChoiceAButton->SetVisibility(ChoiceVisibility);
	ChoiceBButton->SetVisibility(ChoiceVisibility);

	NextButton->SetVisibility(
		bVisible
		? ESlateVisibility::Collapsed
		: ESlateVisibility::Visible
	);
}