// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AmmoPickupWidget.generated.h"

class UTextBlock;
class UImage;
class AAmmo;

/**
 * 
 */
UCLASS()
class SHOOTER_API UAmmoPickupWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Setup(AAmmo* InItem);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UImage* AmmoIcon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (BindWidget))
	UTextBlock* AmmoAmountText;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	AAmmo* AmmoReference;

};
