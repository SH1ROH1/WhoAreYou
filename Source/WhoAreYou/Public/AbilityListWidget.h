
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AbilityListWidget.generated.h"

class APlayerCharacter;
class UTextBlock;
class UVerticalBox;

UCLASS()
class WHOAREYOU_API UAbilityListWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Abilities UI")
	void RefreshList();

	UFUNCTION(BlueprintCallable, Category = "Abilities UI")
	void SetPlayerCharacter(APlayerCharacter* Character);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> ListBox;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> HeaderText;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Abilities UI")
	int32 FontSize = 16;

private:


	UPROPERTY()
	TObjectPtr<APlayerCharacter> PlayerCharacter;

	void AddSection(const FString& Title);
	void AddEntry(const FString& Name, const FString& ActionName, bool bShowKey = true);
	FString GetActionKeys(FName ActionName) const;
};
