// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShooterOverlay.generated.h"

/**
 * 
 */
UCLASS()
class SHOOTER_API UShooterOverlay : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintImplementableEvent)
	void UpdateInventorySlot(int32 SlotID);

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHealthBar(float Percent);
};
