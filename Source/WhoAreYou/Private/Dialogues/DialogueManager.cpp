#include "Dialogues/DialogueManager.h"
#include "Dialogues/DialogueDataAsset.h"
#include "Dialogues/DialogueWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

ADialogueManager::ADialogueManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ADialogueManager::BeginPlay()
{
	Super::BeginPlay();
}

ADialogueManager* ADialogueManager::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject)
	{
		return nullptr;
	}

	UWorld* World = WorldContextObject->GetWorld();

	if (!World)
	{
		return nullptr;
	}

	return Cast<ADialogueManager>(UGameplayStatics::GetActorOfClass(World, ADialogueManager::StaticClass()));
}

void ADialogueManager::StartDialogue(UDialogueDataAsset* DialogueData)
{
	if (!DialogueData || DialogueData->Nodes.IsEmpty() || !DialogueWidgetClass)
	{
		return;
	}

	EndDialogue();

	CurrentDialogue = DialogueData;
	CurrentNodeIndex = 0;

	UWorld* World = GetWorld();

	if (!World)
	{
		return;
	}

	DialogueWidget = CreateWidget<UDialogueWidget>(World, DialogueWidgetClass);

	if (!DialogueWidget)
	{
		CurrentDialogue = nullptr;
		CurrentNodeIndex = -1;
		return;
	}

	DialogueWidget->AddToViewport(100);

	if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(DialogueWidget->TakeWidget());
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		PC->SetInputMode(InputMode);
	}

	ShowCurrentNode();
}

void ADialogueManager::ShowCurrentNode()
{
	if (!CurrentDialogue || !CurrentDialogue->Nodes.IsValidIndex(CurrentNodeIndex) || !DialogueWidget)
	{
		EndDialogue();
		return;
	}

	DialogueWidget->ShowNode(CurrentDialogue->Nodes[CurrentNodeIndex]);
}

void ADialogueManager::Next()
{
	if (!CurrentDialogue || !CurrentDialogue->Nodes.IsValidIndex(CurrentNodeIndex))
	{
		return;
	}

	const FDialogueNode& Node = CurrentDialogue->Nodes[CurrentNodeIndex];

	if (Node.Type == EDialogueNodeType::Choice)
	{
		return;
	}

	if (Node.NextNode < 0)
	{
		EndDialogue();
		return;
	}

	CurrentNodeIndex = Node.NextNode;
	ShowCurrentNode();
}

void ADialogueManager::SelectChoice(bool bChoiceB)
{
	if (!CurrentDialogue || !CurrentDialogue->Nodes.IsValidIndex(CurrentNodeIndex))
	{
		return;
	}

	const FDialogueNode& Node = CurrentDialogue->Nodes[CurrentNodeIndex];

	if (Node.Type != EDialogueNodeType::Choice)
	{
		return;
	}

	OnChoiceSelected.Broadcast(bChoiceB);

	CurrentNodeIndex = bChoiceB ? Node.ChoiceBNextNode : Node.ChoiceANextNode;

	if (CurrentNodeIndex < 0)
	{
		EndDialogue();
		return;
	}

	ShowCurrentNode();
}

void ADialogueManager::EndDialogue()
{
	if (!CurrentDialogue)
	{
		return;
	}

	if (DialogueWidget)
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}

	CurrentDialogue = nullptr;
	CurrentNodeIndex = -1;

	if (UWorld* World = GetWorld())
	{
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(World, 0))
		{
			PC->bShowMouseCursor = false;
			PC->SetInputMode(FInputModeGameOnly());
		}
	}

	OnDialogueEnded.Broadcast();
}

bool ADialogueManager::IsDialogueActive() const
{
	return CurrentDialogue != nullptr;
}