// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "AmmoType.h"
#include "WeaponType.h"
#include "Engine/DataTable.h"
#include "Weapon.generated.h"

class USoundCue;

USTRUCT(BlueprintType)
struct FWeaponDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EAmmoType AmmoType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DefaultWeaponAmmo;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MagazineCapacity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* EquipSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USkeletalMesh* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* InventoryIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstance* MaterialInstance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 MaterialIndex;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ClipBoneName;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName ReloadMontageSection;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UAnimInstance> AnimBP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsMiddle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsBottom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AutomaticFireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* FireSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName BoneToHide;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxSlideDisplacement;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UCurveFloat* SlideDisplacementCurve;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxRecoilRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanAutoFire;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Damage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float HeadShotDamage;
};

/**
 * 
 */
UCLASS()
class SHOOTER_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void ThrowWeapon();
	void DecrementAmmo();
	void ReloadAmmo(int32 Amount);
	FTransform GetClipBoneTransform();
	void SetCharacterInventoryFull(bool bIsFull) override;
	void PlayEquipSound();
	void Fire();
	
	UFUNCTION(BlueprintCallable)
	bool GetWeaponRecord(FWeaponDataTable& Record);

protected:
	void StopFalling();
	virtual void BeginPlay() override;

private:
	void StartSlideTimer();
	void UpdateSlideDisplacement();
	void FinishMovingSlide();

	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime = 0.7f;
	bool bFalling = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 AmmoCount = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	int32 MagazineCapacity = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EWeaponType WeaponType = EWeaponType::EWT_SubmachineGun;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	EAmmoType AmmoType = EAmmoType::EAT_9mm;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ReloadMontageSection = "Reload SMG";

	/* For reloading animation */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	bool bMovingClip = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Properties", meta = (AllowPrivateAccess = "true"))
	FName ClipBoneName = "smg_clip";
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* WeaponDataTable;

	FWeaponDataTable* WeaponRecord;

	float AutomaticFireRate = 0.1f;

	FTimerHandle SlideTimer;

	float CurrentSlideDisplacement = 0.f;

	float CurrentRecoilRotation = 0.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pistol", meta = (AllowPrivateAccess = "true"))
	bool bMovingSlide = false;


public:
	FORCEINLINE int32 GetAmmoAmount() const { return AmmoCount; }
	FORCEINLINE int32 GetMagazingCapacity() const { return MagazineCapacity; }
	FORCEINLINE EAmmoType GetAmmoType() const { return AmmoType; }
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE FName GetReloadSectionName() const { return ReloadMontageSection; }
	FORCEINLINE void SetClipIsMoving(bool Move) { bMovingClip = Move; }
	FORCEINLINE bool CanReload() const { return MagazineCapacity > AmmoCount; }
	FORCEINLINE float GetAutomaticFireRate() const { return AutomaticFireRate; }
	FORCEINLINE USoundBase* GetFireSound() const { return WeaponRecord->FireSound; }
	FORCEINLINE UParticleSystem* GetMuzzleFlash() const { return WeaponRecord->MuzzleFlash; }
	FORCEINLINE float GetDamage() const { return WeaponRecord == nullptr ? 0 : WeaponRecord->Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return WeaponRecord == nullptr ? 0 : WeaponRecord->HeadShotDamage; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetCrosshairsMiddle() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->CrosshairsMiddle; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetCrosshairsLeft() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->CrosshairsLeft; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetCrosshairsRight() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->CrosshairsRight; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetCrosshairsTop() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->CrosshairsTop; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetCrosshairsBottom() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->CrosshairsBottom; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetSlideDisplacement() const { return CurrentSlideDisplacement; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE float GetRecoilRotation() const { return CurrentRecoilRotation; }
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE bool CanAutoFire() const { return WeaponRecord == nullptr ? nullptr : WeaponRecord->bCanAutoFire; }

};
