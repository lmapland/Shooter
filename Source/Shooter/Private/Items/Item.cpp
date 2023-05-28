// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Curves/CurveVector.h"

AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	SetRootComponent(ItemMesh);

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	CollisionBox->SetupAttachment(GetRootComponent());
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(GetRootComponent());

	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(GetRootComponent());
}

void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ItemInterp(DeltaTime);
	UpdatePulse();
}

void AItem::BeginPlay()
{
	Super::BeginPlay();

	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);

	SetItemProperties(ItemState);
	InitializeCustomDepth();

	StartPulseTimer();

	if (ItemRarityDataTable)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Getting RarityRecord for %s"), *(UEnum::GetValueAsName(ItemRarity).ToString()));
		RarityRecord = ItemRarityDataTable->FindRow<FItemRarityTable>(UEnum::GetValueAsName(ItemRarity), FString("ItemBeginPlay"));
		/*if (RarityRecord && MaterialInstance)
		{
			DynamicMaterialInstance = UMaterialInstanceDynamic::Create(MaterialInstance, this);
			DynamicMaterialInstance->SetVectorParameterValue(TEXT("FresnelColor"), RarityRecord->GlowColor);
			ItemMesh->SetMaterial(MaterialIndex, DynamicMaterialInstance);
			SetGlowMaterialEnabled(true);
		}*/
	}
}

void AItem::SetPickupWidgetVisibility(bool IsVisible)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(IsVisible);
	}
}

void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	SetItemProperties(ItemState);
}

void AItem::StartItemCurve(AShooterCharacter* Char)
{
	Character = Char;
	InterpLocIndex = Character->GetInterpLocationIndex();
	Character->UseInterpLocation(InterpLocIndex);

	ItemInterpStartLocation = GetActorLocation();
	bInterping = true;
	SetItemState(EItemState::EIS_EquipInterping);
	GetWorldTimerManager().ClearTimer(PulseTimer);

	PlayPickupSound();

	GetWorldTimerManager().SetTimer(ItemInterpTimer, this, &AItem::FinishInterping, ZCurveTime);

	InterpInitialYawOffset = GetActorRotation().Yaw - Character->GetFollowCamera()->GetComponentRotation().Yaw;
	bCanChangeCustomDepth = false;
}

void AItem::PlayPickupSound()
{
	//if (!GetWorldTimerManager().IsTimerActive(PickupSoundTimer))
	//{
		//GetWorldTimerManager().SetTimer(PickupSoundTimer, this, &AItem::ResetSoundTimers, PickupSoundTimeout);
	PlaySound(PickupSound);
	//}
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(OtherActor))
	{
		Shooter->IncremementOverlappedItemCount(1);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;
	if (AShooterCharacter* Shooter = Cast<AShooterCharacter>(OtherActor))
	{
		Shooter->IncremementOverlappedItemCount(-1);
		Shooter->UnHighlightSlot();
	}
}

void AItem::SetItemProperties(EItemState State)
{
	switch (State)
	{
	//case EItemState::EIS_PickedUp:
	case EItemState::EIS_Pickup:
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		break;
	case EItemState::EIS_Equipped:
		if (PickupWidget) PickupWidget->SetVisibility(false);

		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_Falling:
		ItemMesh->SetSimulatePhysics(true);
		ItemMesh->SetEnableGravity(true);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_EquipInterping:
		if (PickupWidget) PickupWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(true);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	case EItemState::EIS_PickedUp:
		if (PickupWidget) PickupWidget->SetVisibility(false);
		ItemMesh->SetSimulatePhysics(false);
		ItemMesh->SetEnableGravity(false);
		ItemMesh->SetVisibility(false);
		ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AItem::ItemInterp(float DeltaTime)
{
	if (!bInterping) return;

	if (Character && ItemZCurve)
	{
		const float ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);

		FVector ItemLocation = ItemInterpStartLocation;
		const FVector CameraInterpLocation{ GetInterpLocation() };

		ItemLocation.Z += ItemZCurve->GetFloatValue(ElapsedTime) * FVector(0.f, 0.f, (CameraInterpLocation - ItemInterpStartLocation).Z).Size();
		ItemLocation.X = FMath::FInterpTo(GetActorLocation().X, CameraInterpLocation.X, DeltaTime, 30.f);
		ItemLocation.Y = FMath::FInterpTo(GetActorLocation().Y, CameraInterpLocation.Y, DeltaTime, 30.f);
		SetActorLocation(ItemLocation, true, nullptr, ETeleportType::TeleportPhysics);

		const FRotator CameraRotation{ Character->GetFollowCamera()->GetComponentRotation() };
		FRotator ItemRotation{ 0.f, CameraRotation.Yaw + InterpInitialYawOffset, 0.f };
		SetActorRotation(ItemRotation, ETeleportType::TeleportPhysics);

		if (ItemScaleCurve)
		{
			const float ScaleCurveValue = ItemScaleCurve->GetFloatValue(ElapsedTime);
			SetActorScale3D(FVector(ScaleCurveValue, ScaleCurveValue, ScaleCurveValue));
		}
	}
}

void AItem::EnableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(true);
	}
}

void AItem::DisableCustomDepth()
{
	if (bCanChangeCustomDepth)
	{
		ItemMesh->SetRenderCustomDepth(false);
	}
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}

void AItem::OnConstruction(const FTransform& Transform)
{
}

void AItem::UpdatePulse()
{
	if (ItemState != EItemState::EIS_Pickup && ItemState != EItemState::EIS_EquipInterping) return;
	float ElapsedTime{};
	FVector CurveValue{};

	if (ItemState == EItemState::EIS_Pickup && PulseCurve)
	{
		ElapsedTime = GetWorldTimerManager().GetTimerElapsed(PulseTimer);
		CurveValue = PulseCurve->GetVectorValue(ElapsedTime);
	}
	else if (ItemState == EItemState::EIS_EquipInterping && InterpPulseCurve)
	{
		ElapsedTime = GetWorldTimerManager().GetTimerElapsed(ItemInterpTimer);
		CurveValue = InterpPulseCurve->GetVectorValue(ElapsedTime);
	}

	if (DynamicMaterialInstance)
	{
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowAmount"), CurveValue.X * GlowAmount);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("FresnelExp"), CurveValue.Y * FresnelExponent);
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("BaseReflectFraction"), CurveValue.Z * FresnelReflectFraction);
	}
}

void AItem::SetGlowMaterialEnabled(bool bValue)
{
	if (DynamicMaterialInstance)
	{
		// 0 if true
		float BlendAlpha = bValue == true ? 0 : 1;
		DynamicMaterialInstance->SetScalarParameterValue(TEXT("GlowBlendAlpha"), BlendAlpha);
	}
}

void AItem::SetCharacterInventoryFull(bool bIsFull)
{
}

// TODO: make this a virtual function and have the child classes override it to return the correct thing
// overall this is hacky anyway; Item shouldn't need to know that the returned has a scene component
// it should just receive the location... but then how would it free the location after?
FVector AItem::GetInterpLocation()
{
	if (Character == nullptr) return FVector(0.f);
	switch (ItemType)
	{
	case EItemType::EIT_Ammo:
		return Character->GetInterpLocation(InterpLocIndex).SceneComponent->GetComponentLocation();
		break;
	case EItemType::EIT_Weapon:
		return Character->GetInterpLocation(0).SceneComponent->GetComponentLocation();
		break;
	default:
		break;
	}

	return FVector(0.f);
}

void AItem::FinishInterping()
{
	bInterping = false;
	SetItemState(EItemState::EIS_Pickup);
	if (Character)
	{
		Character->GetPickupItem(this);
		Character->FreeInterpLocation(InterpLocIndex);
	}
	SetActorScale3D(FVector(1.f));
	SetGlowMaterialEnabled(false);
	bCanChangeCustomDepth = true;
	DisableCustomDepth();
}

void AItem::PlaySound(USoundCue* SoundToPlay)
{
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
	}
}

void AItem::ResetPulseTimer()
{
	StartPulseTimer();
}

void AItem::StartPulseTimer()
{
	if (ItemState == EItemState::EIS_Pickup)
	{
		GetWorldTimerManager().SetTimer(PulseTimer, this, &AItem::ResetPulseTimer, PulseCurveTime);
	}
}
