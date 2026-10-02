#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "DialogueDataAsset.generated.h"

UENUM(BlueprintType)
enum class EDialogueNodeType : uint8
{
	Line,
	Choice
};

USTRUCT(BlueprintType)
struct FDialogueNode
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EDialogueNodeType Type = EDialogueNodeType::Line;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MultiLine = true))
	FText Text;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ChoiceA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText ChoiceB;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 NextNode = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChoiceANextNode = -1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChoiceBNextNode = -1;
};

UCLASS(BlueprintType)
class WHOAREYOU_API UDialogueDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FDialogueNode> Nodes;
};