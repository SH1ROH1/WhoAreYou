#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DialogueManager.generated.h"

class UDialogueDataAsset;
class UDialogueWidget;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDialogueChoiceSelected, bool, bChoiceB);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDialogueEnded);

UCLASS()
class WHOAREYOU_API ADialogueManager : public AActor
{
	GENERATED_BODY()

public:
	ADialogueManager();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintPure, Category = "Dialogue", meta = (WorldContext = "WorldContextObject"))
	static ADialogueManager* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void StartDialogue(UDialogueDataAsset* DialogueData);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void Next();

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void SelectChoice(bool bChoiceB);

	UFUNCTION(BlueprintCallable, Category = "Dialogue")
	void EndDialogue();

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	bool IsDialogueActive() const;

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueChoiceSelected OnChoiceSelected;

	UFUNCTION(BlueprintPure, Category = "Dialogue")
	UDialogueWidget* GetDialogueWidget()
	{
		return DialogueWidget;
	}

	UPROPERTY(BlueprintAssignable, Category = "Dialogue")
	FOnDialogueEnded OnDialogueEnded;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue")
	TSubclassOf<UDialogueWidget> DialogueWidgetClass;

private:
	UPROPERTY()
	TObjectPtr<UDialogueWidget> DialogueWidget;

	UPROPERTY()
	TObjectPtr<UDialogueDataAsset> CurrentDialogue;

	int32 CurrentNodeIndex = -1;

	void ShowCurrentNode();
};