// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/AmmoPickupWidget.h"
#include "Components/TextBlock.h"
#include "Items/Ammo.h"
#include "Components/Image.h"

void UAmmoPickupWidget::Setup(AAmmo* InItem)
{
	AmmoReference = InItem;

	/* Set the Item Name and Count */
	if (AmmoIcon) AmmoIcon->SetBrushFromTexture(AmmoReference->GetAmmoIconTexture());
	if (AmmoAmountText) AmmoAmountText->SetText(FText::FromString(FString::FromInt(InItem->GetItemCount())));
}
