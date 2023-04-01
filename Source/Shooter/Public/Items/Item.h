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

class UBoxComponent;
class UWidgetComponent;
class USphereComponent;

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:
	AItem();
	virtual void Tick(float DeltaTime) override;
	void SetPickupWidgetVisibility(bool IsVisible);

protected:
	virtual void BeginPlay() override;

	/* The Player Character must be within the sphere's radius to even be able to view the Item Details (PickupWidget) */
	UFUNCTION()
	void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	void SetActiveStars();

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

	/* To help the frontend know how many spheres to display */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Properties", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;

public:
	FORCEINLINE FString GetItemName() const { return ItemName; }
	FORCEINLINE int32 GetItemCount() const { return ItemCount; }
	FORCEINLINE TArray<bool> GetActiveStars() const { return ActiveStars; }
};
