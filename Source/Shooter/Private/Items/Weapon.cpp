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

void AWeapon::StopFalling()
{
	bFalling = false;
	SetItemState(EItemState::EIS_Pickup);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		if (PickupWidget->GetUserWidgetObject()) // if this is not set this code will crash
		{
			PickupWidget->SetVisibility(false);
			UPickupWidget* ItemDetailsPanel = Cast<UPickupWidget>(PickupWidget->GetUserWidgetObject());
			ItemDetailsPanel->Setup(this);
		}
		else // this is a common bug; let the user know that they need to do something
		{
			UE_LOG(LogTemp, Warning, TEXT("There is NO WidgetClass Set in PickupWidget > User Interface > Widget Class"));
		}
	}
}
