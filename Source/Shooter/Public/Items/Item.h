// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged = 0 UMETA(DisplayName = "Damaged"),
	EIR_Common = 1 UMETA(DisplayName = "Common"),
	EIR_Uncommon = 2 UMETA(DisplayName = "Uncommon"),
	EIR_Rare = 3 UMETA(DisplayName = "Rare"),
	EIR_Legendary = 4 UMETA(DisplayName = "Legendary")
};
/* Add ability to iterate over the enums */
ENUM_RANGE_BY_FIRST_AND_LAST(EItemRarity, EItemRarity::EIR_Damaged, EItemRarity::EIR_Legendary);

UENUM(BlueprintType)
enum class EItemState : uint8
{
	EIS_Equipped = 0 UMETA(DisplayName = "Equipped"),
	EIS_Pickup = 1 UMETA(DisplayName = "Pickup"),
	EIS_EquipInterping = 2 UMETA(DisplayName = "EquipInterping"),
	EIS_PickedUp = 3 UMETA(DisplayName = "PickedUp"),
	EIS_Falling = 4 UMETA(DisplayName = "Falling")

};

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class AShooterCharacter;

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:
	AItem();
	virtual void Tick(float DeltaTime) override;
	void SetPickupWidgetVisibility(bool IsVisible);
	void SetItemState(EItemState State);
	void StartItemCurve(AShooterCharacter* Char);

protected:
	virtual void BeginPlay() override;

	/* The Player Character must be within the sphere's radius to even be able to view the Item Details (PickupWidget) */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	void SetItemProperties(EItemState State);
	void ItemInterp(float DeltaTime);

private:
	void FinishInterping();

	/* Components of an Item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;

	/* Details of an item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName = "Default";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity = EItemRarity::EIR_Common;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState = EItemState::EIS_Pickup;
	
	/* Setting the behavior when picking up items */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemZCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	FVector ItemInterpStartLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	FVector CameraTargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	bool bInterping = false;

	FTimerHandle ItemInterpTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	float ZCurveTime = 0.7f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	AShooterCharacter* Character;

	//float ItemInterpX = 0.f;
	//float ItemInterpY = 0.f;

	/* Offset between the camera and the interping item */
	float InterpInitialYawOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;

public:
	FORCEINLINE FString GetItemName() const { return ItemName; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE EItemRarity GetItemRarity() const { return ItemRarity; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
};
