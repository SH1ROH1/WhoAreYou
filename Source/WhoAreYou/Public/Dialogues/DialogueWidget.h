#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DialogueWidget.generated.h"

class UButton;
class UTextBlock;
struct FDialogueNode;
class USoundBase;

UCLASS()
class WHOAREYOU_API UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void ShowNode(const FDialogueNode& Node);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> SpeakerText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DialogueText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> NextButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChoiceAButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> ChoiceBButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChoiceAText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ChoiceBText;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue|Sound")
	TObjectPtr<USoundBase> TypewriterSound;

private:
	FTimerHandle TypewriterTimer;

	FString FullText;
	int32 CurrentCharacter = 0;
	bool bIsTyping = false;
	bool bCurrentNodeIsChoice = false;

	void StartTypewriter(const FText& Text);
	void TypewriterTick();
	void FinishTypewriter();

	UFUNCTION()
	void OnNextClicked();

	UFUNCTION()
	void OnChoiceAClicked();

	UFUNCTION()
	void OnChoiceBClicked();

	void SetChoicesVisible(bool bVisible);
};