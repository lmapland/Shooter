// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapon.h"
#include "Components/WidgetComponent.h"
#include "Widgets/PickupWidget.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = true;
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep the weapon upright
	if (GetItemState() == EItemState::EIS_Falling && bFalling)
	{
		const FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
		GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);
	}

	if (bMovingSlide)
	{
		UpdateSlideDisplacement();
	}
}

void AWeapon::ThrowWeapon()
{
	FRotator MeshRotation{ 0.f, GetItemMesh()->GetComponentRotation().Yaw, 0.f };
	GetItemMesh()->SetWorldRotation(MeshRotation, false, nullptr, ETeleportType::TeleportPhysics);

	// Direction in which we throw the weapon
	FVector ImpulseDirection = GetItemMesh()->GetRightVector().RotateAngleAxis(-20, GetItemMesh()->GetForwardVector());

	float RandomRotation = FMath::FRandRange(5.f, 60.f);
	ImpulseDirection = ImpulseDirection.RotateAngleAxis(RandomRotation, FVector(0.f, 0.f, 1.f));
	ImpulseDirection *= 500.f;
	GetItemMesh()->AddImpulse(ImpulseDirection);
	bFalling = true;
	GetWorldTimerManager().SetTimer(ThrowWeaponTimer, this, &AWeapon::StopFalling, ThrowWeaponTime);
	SetGlowMaterialEnabled(true);
}

void AWeapon::DecrementAmmo()
{
	AmmoCount = FMath::Clamp(AmmoCount - 1, 0, 100'000);
}

void AWeapon::ReloadAmmo(int32 Amount)
{
	AmmoCount = FMath::Clamp(AmmoCount + Amount, 0, MagazineCapacity);
}

FTransform AWeapon::GetClipBoneTransform()
{
	return GetItemMesh()->GetBoneTransform(GetItemMesh()->GetBoneIndex(ClipBoneName));
}

void AWeapon::SetCharacterInventoryFull(bool bIsFull)
{
	if (bIsFull)
	{
		UPickupWidget* Pickup = Cast<UPickupWidget>(PickupWidget->GetWidget());
		if (Pickup)
		{
			Pickup->UpdatePickupText();
		}
	}
}

void AWeapon::PlayEquipSound()
{
	if (WeaponRecord)
	{
		PlaySound(WeaponRecord->EquipSound);
	}
}

void AWeapon::Fire()
{
	DecrementAmmo();
	StartSlideTimer();
}

bool AWeapon::GetWeaponRecord(FWeaponDataTable& Record)
{
	if (WeaponRecord)
	{
		Record = *WeaponRecord;
		return true;
	}
	return false;
}

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
	StartPulseTimer();
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (WeaponDataTable)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Getting RarityRecord for %s"), *(UEnum::GetValueAsName(ItemRarity).ToString()));
		WeaponRecord = WeaponDataTable->FindRow<FWeaponDataTable>(UEnum::GetValueAsName(WeaponType), FString("WeaponBeginPlay"));
		if (WeaponRecord)
		{
			PickupSound = WeaponRecord->PickupSound;
			GetItemMesh()->SetSkeletalMesh(WeaponRecord->ItemMesh);
			InventoryIcon = WeaponRecord->InventoryIcon;
			ItemName = WeaponRecord->ItemName;
			AmmoType = WeaponRecord->AmmoType;
			MaterialInstance = WeaponRecord->MaterialInstance;
			MaterialIndex = WeaponRecord->MaterialIndex;
			ClipBoneName = WeaponRecord->ClipBoneName;
			ReloadMontageSection = WeaponRecord->ReloadMontageSection;
			GetItemMesh()->SetAnimInstanceClass(WeaponRecord->AnimBP);
			AutomaticFireRate = WeaponRecord->AutomaticFireRate;

			if (RarityRecord && WeaponRecord->MaterialInstance && GetItemMesh())
			{
				DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
				DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), RarityRecord->GlowColor);
				GetItemMesh()->SetMaterial(MaterialIndex, DynamicMaterialInstance);
				SetGlowMaterialEnabled(true);
				GetItemMesh()->SetCustomDepthStencilValue(RarityRecord->CustomDepthStencil);
			}

			if (WeaponRecord->BoneToHide != FName(""))
			{
				GetItemMesh()->HideBoneByName(WeaponRecord->BoneToHide, EPhysBodyOp::PBO_None);
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("WeaponRecord not found"));
		}
	}

	if (PickupWidget)
	{
		if (PickupWidget->GetUserWidgetObject()) // if this is not set this code will crash
		{
			PickupWidget->SetVisibility(false);
			UPickupWidget* ItemDetailsPanel = Cast<UPickupWidget>(PickupWidget->GetUserWidgetObject());
			ItemDetailsPanel->Setup(this);
			if (RarityRecord)
			{
				ItemDetailsPanel->SetRightUpperBG(GetLightColor());
				ItemDetailsPanel->SetRightLowerBG(GetDarkColor());
			}
			if (WeaponRecord)
			{
				ItemDetailsPanel->UpdateAmmoImage(WeaponRecord->AmmoType);
			}
		}
		else // this is a common bug; let the user know that they need to do something
		{
			UE_LOG(LogTemp, Warning, TEXT("There is NO WidgetClass Set in PickupWidget > User Interface > Widget Class"));
		}
	}
}

void AWeapon::StartSlideTimer()
{
	if (WeaponType != EWeaponType::EWT_Pistol || bMovingSlide) return;

	bMovingSlide = true;

	GetWorldTimerManager().SetTimer(SlideTimer, this, &AWeapon::FinishMovingSlide, WeaponRecord->AutomaticFireRate);
}

void AWeapon::UpdateSlideDisplacement()
{
	if (bMovingSlide && WeaponRecord->SlideDisplacementCurve)
	{
		const float ElapsedTime{ GetWorldTimerManager().GetTimerElapsed(SlideTimer) / WeaponRecord->AutomaticFireRate };
		const float CurveValue{ WeaponRecord->SlideDisplacementCurve->GetFloatValue(ElapsedTime) };
		CurrentSlideDisplacement = CurveValue * WeaponRecord->MaxSlideDisplacement;
		CurrentRecoilRotation = CurveValue * WeaponRecord->MaxRecoilRotation;
	}
}

void AWeapon::FinishMovingSlide()
{
	bMovingSlide = false;
}
