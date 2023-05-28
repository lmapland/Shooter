// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/DataTable.h"
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

UENUM(BlueprintType)
enum class EItemType : uint8
{
	EIT_Ammo = 0 UMETA(DisplayName = "Ammo"),
	EIT_Weapon = 1 UMETA(DisplayName = "Weapon")
};

USTRUCT(BlueprintType)
struct FItemRarityTable : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor GlowColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor LightColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor DarkColor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 NumberOfStars;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UTexture2D* IconBackground;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 CustomDepthStencil;
};


class UBoxComponent;
class UWidgetComponent;
class USphereComponent;
class AShooterCharacter;
class USoundCue;
class UCurveVector;
class UDataTable;

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
	void PlayPickupSound();
	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	void SetGlowMaterialEnabled(bool bValue);
	virtual void SetCharacterInventoryFull(bool bIsFull);

protected:
	virtual void BeginPlay() override;

	/* The Player Character must be within the sphere's radius to even be able to view the Item Details (PickupWidget) */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	virtual void SetItemProperties(EItemState State);
	void ItemInterp(float DeltaTime);

	virtual void InitializeCustomDepth();
	virtual void OnConstruction(const FTransform& Transform) override;
	void UpdatePulse();
	void StartPulseTimer();
	void PlaySound(USoundCue* SoundToPlay);

	FVector GetInterpLocation();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UWidgetComponent* PickupWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	USoundCue* PickupSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemType ItemType = EItemType::EIT_Weapon;

	FItemRarityTable* RarityRecord;
	
	/* Dynamic Instance we can change at runtime */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstanceDynamic* DynamicMaterialInstance;
	
	// Because an item may have more than 1 material and this indicates which material needs changed
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 MaterialIndex = 0;

	/* Material instance used with the Dyanmic Material */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UMaterialInstance* MaterialInstance;

	/* Details of an item */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	FString ItemName = "Default";
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity = EItemRarity::EIR_Common;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	EItemState ItemState = EItemState::EIS_Pickup;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UTexture2D* InventoryIcon;

private:
	void FinishInterping();
	void ResetPulseTimer();

	/* Components of an Item */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* CollisionBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	USphereComponent* AreaSphere;

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

	/* Offset between the camera and the interping item */
	float InterpInitialYawOffset = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Pickup", meta = (AllowPrivateAccess = "true"))
	UCurveFloat* ItemScaleCurve;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 InterpLocIndex = 0;

	bool bCanChangeCustomDepth = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* PulseCurve;

	FTimerHandle PulseTimer;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float PulseCurveTime = 5.f;

	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float GlowAmount = 150.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelExponent = 3.f;
	
	UPROPERTY(VisibleAnywhere, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	float FresnelReflectFraction = 4.f;

	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	UCurveVector* InterpPulseCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex = 0;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* ItemRarityDataTable;

	// This stuff is only useful in the case that I create a singleton that is solely responsible for playing sounds. Then it can manage this info
	//FTimerHandle PickupSoundTimer;
	//float PickupSoundTimeout = 0.5f;
	//FTimerHandle EquipSoundTimer;
	//float EquipSoundTimeout = 0.5f;

public:
	FORCEINLINE FString GetItemName() const { return ItemName; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE EItemRarity GetItemRarity() const { return ItemRarity; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	FORCEINLINE EItemType GetItemType() const { return ItemType; }
	FORCEINLINE USkeletalMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE UBoxComponent* GetCollisionBox() const { return CollisionBox; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	FORCEINLINE FLinearColor GetGlowColor() const { return RarityRecord == nullptr ? FLinearColor::Red : RarityRecord->GlowColor; }
	FORCEINLINE FLinearColor GetLightColor() const { return RarityRecord == nullptr ? FLinearColor::Red : RarityRecord->LightColor; }
	FORCEINLINE FLinearColor GetDarkColor() const { return RarityRecord == nullptr ? FLinearColor::Red : RarityRecord->DarkColor; }
	FORCEINLINE int32 GetNumberOfStars() const { return RarityRecord == nullptr ? -1 : RarityRecord->NumberOfStars; }
	FORCEINLINE int32 GetCustomDepthStencil() const { return RarityRecord == nullptr ? -1 : RarityRecord->CustomDepthStencil; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE UTexture2D* GetIconBackground() const { return RarityRecord == nullptr ? nullptr : RarityRecord->IconBackground; }
};
