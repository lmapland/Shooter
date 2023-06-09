// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/PickupWidget.h"
#include "Components/TextBlock.h"
#include "Items/Item.h"
#include "Components/Image.h"

void UPickupWidget::Setup(AItem* InItem)
{
	ItemReference = InItem;
	UpdateAmmoImage(AmmoType);

	/* Set the Item Name and Count */
	if (ItemNameText) ItemNameText->SetText(FText::FromString(InItem->GetItemName()));
	if (ItemAmountText) ItemAmountText->SetText(FText::FromString(FString::FromInt(InItem->GetItemCount())));

	/* Set the Stars */
	Stars.Add(Star1Icon);
	Stars.Add(Star2Icon);
	Stars.Add(Star3Icon);
	Stars.Add(Star4Icon);
	Stars.Add(Star5Icon);

	for (EItemRarity Rarity : TEnumRange<EItemRarity>())
	{
		if (Rarity <= InItem->GetItemRarity()) Stars[(uint8)Rarity]->SetOpacity(1.f);
		else Stars[(uint8)Rarity]->SetOpacity(0.25f);
	}
}

void UPickupWidget::UpdatePickupText()
{
	if (IconLabelText)
	{
		IconLabelText->SetText(FText::FromString("Swap"));
	}
}

void UPickupWidget::SetRightUpperBG(FLinearColor Color)
{
	if (RightUpperBG)
	{
		RightUpperBG->SetColorAndOpacity(Color);
	}
}

void UPickupWidget::SetRightLowerBG(FLinearColor Color)
{
	if (RightLowerBG)
	{
		RightLowerBG->SetColorAndOpacity(Color);
	}
}

/*void UPickupWidget::SetAmmoType(EAmmoType Ammo)
{
	AmmoType = Ammo;
	UpdateAmmoImage();
}*/
