// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PickupWidget.generated.h"

class UTextBlock;
class UImage;
class AItem;

/**
 * 
 */
UCLASS()
class SHOOTER_API UPickupWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Setup(AItem* InItem);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* ItemAmountText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AItem* ItemReference;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* Star1Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* Star2Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* Star3Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* Star4Icon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* Star5Icon;

private:
	TArray<UImage*> Stars;
};
