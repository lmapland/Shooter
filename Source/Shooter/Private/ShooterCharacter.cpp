// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "particles/ParticleSystemComponent.h"
#include "Items/Item.h"
#include "Items/Weapon.h"
#include "Items/Ammo.h"

// Sets default values
AShooterCharacter::AShooterCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = TargetArmLength;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->SocketOffset = CameraOffset;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom);

	// Don't rotate when the controller rotates
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	//GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GroundFriction = 2.f;

	HandSceneComponent = CreateDefaultSubobject<USceneComponent>("HandSceneComponent");

	WeaponInterpComp = CreateDefaultSubobject<USceneComponent>(TEXT("Weapon InterpolationComponent"));
	WeaponInterpComp->SetupAttachment(GetFollowCamera());
	InterpComp1 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 1"));
	InterpComp1->SetupAttachment(GetFollowCamera());
	InterpComp2 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 2"));
	InterpComp2->SetupAttachment(GetFollowCamera());
	InterpComp3 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 3"));
	InterpComp3->SetupAttachment(GetFollowCamera());
	InterpComp4 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 4"));
	InterpComp4->SetupAttachment(GetFollowCamera());
	InterpComp5 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 5"));
	InterpComp5->SetupAttachment(GetFollowCamera());
	InterpComp6 = CreateDefaultSubobject<USceneComponent>(TEXT("Interp Component 6"));
	InterpComp6->SetupAttachment(GetFollowCamera());

	MinZoomLength = 100.f;
	MaxZoomLength = 600.f;
	ZoomInOutAmount = 30.f;
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (FollowCamera)
	{
		CameraDefaultFOV = FollowCamera->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}

	EquipWeapon(SpawnDefaultWeapon());

	PlayerController = Cast<APlayerController>(GetController());

	if (PlayerController)
	{
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer());
		if (Subsystem)
		{
			Subsystem->AddMappingContext(CharMappingContext, 0);
		}
	}

	InitializeAmmoMap();
	GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
	InitializeInterpLocations();
}

void AShooterCharacter::Jump()
{
	if (bCrouching) CrouchButtonPressed();
	else Super::Jump();
}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CameraInterpZoom(DeltaTime);
	CalculateCrosshairSpread(DeltaTime);

	TraceForItems();
	InterpCapsuleHalfHeight(DeltaTime);
}

void AShooterCharacter::TraceForItems()
{
	if (!bShouldTraceForItems) 
	{
		TurnOffPickupWidget();
		return;
	}
	
	FHitResult ItemTraceResult;
	FVector HitLocation;
	if (TraceUnderCrosshairs(ItemTraceResult, HitLocation))
	{
		TraceHitItem = Cast<AItem>(ItemTraceResult.GetActor());
		if (TraceHitItem)
		{
			if (PreviousMousedOverItem != TraceHitItem)
			{
				TurnOffPickupWidget();
				TraceHitItem->SetPickupWidgetVisibility(true);
				PreviousMousedOverItem = TraceHitItem;
			}
		}
		else TurnOffPickupWidget();
	}
	else TurnOffPickupWidget();
}

void AShooterCharacter::TurnOffPickupWidget()
{
	if (PreviousMousedOverItem != nullptr)
	{
		PreviousMousedOverItem->SetPickupWidgetVisibility(false);
		PreviousMousedOverItem = nullptr;
	}
}

/* Shrinks the capsule's half height while elevating the character so they do not stand beneath the ground */
void AShooterCharacter::InterpCapsuleHalfHeight(float DeltaTime)
{
	// negative value if crouching, positive if standing
	CurrentCapsuleHalfHeight = FMath::FInterpTo(GetCapsuleComponent()->GetScaledCapsuleHalfHeight(), TargetCapsuleHalfHeight, DeltaTime, 15.f);

	GetMesh()->AddLocalOffset(FVector(0.f, 0.f, -(CurrentCapsuleHalfHeight - GetCapsuleComponent()->GetScaledCapsuleHalfHeight())));
	GetCapsuleComponent()->SetCapsuleHalfHeight(CurrentCapsuleHalfHeight);
}

void AShooterCharacter::Aim()
{
	bAiming = true;

	BaseMouseTurnRate = MouseAimingTurnRate;
	BaseMouseLookUpRate = MouseAimingLookUpRate;

	GetCharacterMovement()->MaxWalkSpeed = AimingMovementSpeed;
}

void AShooterCharacter::StopAiming()
{
	bAiming = false;

	BaseMouseTurnRate = MouseHipTurnRate;
	BaseMouseLookUpRate = MouseHipLookUpRate;

	if (bCrouching) GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
	else GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
}

void AShooterCharacter::PickupAmmo(AAmmo* Ammo)
{
	if (AmmoMap.Find(Ammo->GetAmmoType()))
	{
		AmmoMap[Ammo->GetAmmoType()] += Ammo->GetItemCount();
	}
	else
	{
		AmmoMap.Add(Ammo->GetAmmoType(), Ammo->GetItemCount());
	}

	Ammo->Destroy();
}

void AShooterCharacter::InitializeInterpLocations()
{
	InterpLocations.Add(FInterpLocation{ WeaponInterpComp, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp1, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp2, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp3, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp4, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp5, 0 });
	InterpLocations.Add(FInterpLocation{ InterpComp6, 0 });
}

void AShooterCharacter::CameraInterpZoom(float DeltaTime)
{
	if (bAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);
	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	FollowCamera->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::CalculateCrosshairSpread(float DeltaTime)
{
	/* Calculate Crosshair Velocity */
	FVector2D WalkSpeedRange{ 0.f, 600.f }; // 600.f is assumed to be max walk speed
	FVector2D VelocityMultiplierRange{ 0.f, 1.f };
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;
	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	/* Calculate Crosshair in Air */
	if (GetCharacterMovement()->IsFalling()) CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
	else CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

	/* Calculate Crosshair Aim */
	if (bAiming) CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.6f, DeltaTime, 20.f);
	else CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 20.f);

	/* Calculate Crosshair Shooting */
	if (bFiringBullet) CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.3f, DeltaTime, 60.f);
	else CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);

	CrosshairSpreadMultiplier = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor - CrosshairAimFactor;
}

void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;

	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

bool AShooterCharacter::TraceUnderCrosshairs(FHitResult& OutHitResult, FVector& OutHitLocation)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// Get screen space location of crosshairs
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	// Get world position & direction of crosshairs
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		const FVector Start{ CrosshairWorldPosition };
		const FVector End{ Start + CrosshairWorldDirection * 50'000.f };
		OutHitLocation = End;
		GetWorld()->LineTraceSingleByChannel(OutHitResult, Start, End, ECollisionChannel::ECC_Visibility);

		if (OutHitResult.bBlockingHit)
		{
			OutHitLocation = OutHitResult.Location;
			return true;
		}
	}

	return false;
}

AWeapon* AShooterCharacter::SpawnDefaultWeapon()
{
	if (DefaultWeaponClass)
	{
		return GetWorld()->SpawnActor<AWeapon>(DefaultWeaponClass);
	}
	return nullptr;
}

void AShooterCharacter::EquipWeapon(AWeapon* ToEquip)
{
	//UE_LOG(LogTemp, Warning, TEXT("In EquipWeapon(), weapon state: %i"), (uint8)ToEquip->GetItemState());
	if (ToEquip && ToEquip->GetItemState() == EItemState::EIS_Pickup)
	{
		const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("RightHandSocket"));
		if (HandSocket)
		{
			HandSocket->AttachActor(ToEquip, GetMesh());
		}
		EquippedWeapon = ToEquip;
		EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	}
}

void AShooterCharacter::DropWeapon()
{
	if (EquippedWeapon)
	{
		FDetachmentTransformRules DetachmentRules(EDetachmentRule::KeepWorld, true);
		EquippedWeapon->GetItemMesh()->DetachFromComponent(DetachmentRules);
		EquippedWeapon->SetItemState(EItemState::EIS_Falling);
		EquippedWeapon->ThrowWeapon();
	}
}

void AShooterCharacter::SwapWeapon(AWeapon* WeaponToSwap)
{
	DropWeapon();
	EquipWeapon(WeaponToSwap);
	TraceHitItem = nullptr;
	PreviousMousedOverItem = nullptr;
}

void AShooterCharacter::InitializeAmmoMap()
{
	AmmoMap.Add(EAmmoType::EAT_9mm, StartingAmmo9mm);
	AmmoMap.Add(EAmmoType::EAT_AR, StartingAmmoAR);
}

bool AShooterCharacter::WeaponHasAmmo()
{
	if (EquippedWeapon == nullptr) return false;
	
	return EquippedWeapon->GetAmmoAmount() > 0;
	
}

void AShooterCharacter::FireButtonPressed()
{
	bFireButtonPressed = true;
	if (!WeaponHasAmmo()) return;

	FireWeapon();
}

void AShooterCharacter::FireButtonReleased()
{
	bFireButtonPressed = false;
}

void AShooterCharacter::StartFireTimer()
{
	CombatState = ECombatState::ECS_FireTimerInProgress;

	GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
}

void AShooterCharacter::AutoFireReset()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (!WeaponHasAmmo())
	{
		ReloadWeapon();
	}
	else
	{
		if (bFireButtonPressed) FireWeapon();
	}
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(ZoomInAction, ETriggerEvent::Triggered, this, &AShooterCharacter::ZoomIn);
		EnhancedInputComponent->BindAction(ZoomOutAction, ETriggerEvent::Triggered, this, &AShooterCharacter::ZoomOut);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Move); // wasd
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AShooterCharacter::Look); // mouse

		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &AShooterCharacter::AimingButtonPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &AShooterCharacter::AimingButtonReleased);

		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AShooterCharacter::Jump);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AShooterCharacter::FireButtonPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &AShooterCharacter::FireButtonReleased);

		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Started, this, &AShooterCharacter::InteractStart);
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Completed, this, &AShooterCharacter::Interact);
		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Triggered, this, &AShooterCharacter::ReloadButtonPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &AShooterCharacter::CrouchButtonPressed);
		/*EnhancedInputComponent->BindAction(OpenEverythingMenuAction, ETriggerEvent::Completed, this, &ASlashCharacter::OpenEverythingMenu);

		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Started, this, &ASlashCharacter::Block);
		EnhancedInputComponent->BindAction(BlockAction, ETriggerEvent::Completed, this, &ASlashCharacter::EndBlock);

		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ASlashCharacter::Sprint);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ASlashCharacter::EndSprint);
		*/
	}
}

float AShooterCharacter::GetCrosshairSpreadMultiplier() const
{
	return CrosshairSpreadMultiplier;
}

void AShooterCharacter::IncremementOverlappedItemCount(int8 Amount)
{
	OverlappedItemCount = FMath::Clamp(OverlappedItemCount + Amount, 0, 10'000);

	if (OverlappedItemCount == 0) bShouldTraceForItems = false;
	else bShouldTraceForItems = true;
}

void AShooterCharacter::GetPickupItem(AItem* Item)
{
	Item->PlayEquipSound();

	auto Weapon = Cast<AWeapon>(Item);
	if (Weapon)
	{
		SwapWeapon(Weapon);
	}

	auto Ammo = Cast<AAmmo>(Item);
	if (Ammo)
	{
		PickupAmmo(Ammo);
		
		/* If the EquippedWeapon uses this ammo type and it is empty, reload it */
		if (EquippedWeapon->GetAmmoType() == Ammo->GetAmmoType() && EquippedWeapon->GetAmmoAmount() == 0)
		{
			ReloadWeapon();
		}
	}
}

void AShooterCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();
	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

	AddMovementInput(ForwardDirection, MovementVector.X);
	AddMovementInput(RightDirection, MovementVector.Y);
}

void AShooterCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisValue.X * BaseMouseTurnRate);
	AddControllerPitchInput(LookAxisValue.Y * BaseMouseLookUpRate);
}

void AShooterCharacter::InteractStart(const FInputActionValue& value)
{
	if (TraceHitItem)
	{
		TraceHitItem->StartItemCurve(this);
	}
}

void AShooterCharacter::Interact(const FInputActionValue& value)
{	
}

void AShooterCharacter::ZoomIn()
{
	if (CameraBoom->TargetArmLength > MinZoomLength)
	{
		CameraBoom->TargetArmLength -= ZoomInOutAmount;
	}
}

void AShooterCharacter::ZoomOut()
{
	if (CameraBoom->TargetArmLength <= MaxZoomLength)
	{
		CameraBoom->TargetArmLength += ZoomInOutAmount;
	}
}

void AShooterCharacter::CrouchButtonPressed()
{
	if (GetCharacterMovement()->IsFalling()) return;

	if (!bCrouching)
	{
		GetCharacterMovement()->MaxWalkSpeed = CrouchMovementSpeed;
		bCrouching = true;
		TargetCapsuleHalfHeight = CrouchingCapsuleHalfHeight;
		GetCharacterMovement()->GroundFriction = CrouchGroundFriction;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = BaseMovementSpeed;
		bCrouching = false;
		TargetCapsuleHalfHeight = StandingCapsuleHalfHeight;
		GetCharacterMovement()->GroundFriction = BaseGroundFriction;
	}
}

void AShooterCharacter::FireWeapon()
{
	if (EquippedWeapon == nullptr || !WeaponHasAmmo() || CombatState != ECombatState::ECS_Unoccupied) return;
	
	PlaySound(FireSound);
	SendBullet();
	PlayMontageSection(HipFireMontage, FName("StartFire"));
	EquippedWeapon->DecrementAmmo();

	StartFireTimer();
}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FHitResult CrosshairHitResult;
	bool bCrosshairHit = TraceUnderCrosshairs(CrosshairHitResult, OutBeamLocation);
	if (bCrosshairHit)
	{
		OutBeamLocation = CrosshairHitResult.Location;
	}

	// Perform a second trace, but from the gun barrel
	FHitResult WeaponTraceHit;
	const FVector WeaponTraceStart{ MuzzleSocketLocation };
	const FVector StartToEnd{ OutBeamLocation - MuzzleSocketLocation };
	const FVector WeaponTraceEnd{ MuzzleSocketLocation + StartToEnd * 1.25f };
	GetWorld()->LineTraceSingleByChannel(WeaponTraceHit, WeaponTraceStart, WeaponTraceEnd, ECollisionChannel::ECC_Visibility);

	if (WeaponTraceHit.bBlockingHit)
	{
		OutBeamLocation = WeaponTraceHit.Location;
		return true;
	}
	return false;
}

void AShooterCharacter::PlaySound(USoundBase* SoundToPlay)
{
	if (SoundToPlay)
	{
		UGameplayStatics::PlaySound2D(this, SoundToPlay);
	}
}

void AShooterCharacter::PlayMontageSection(UAnimMontage* Montage, FName SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void AShooterCharacter::PlayCascadeParticles(UParticleSystem* ParticlesToPlay, FVector Location)
{
	if (ParticlesToPlay)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticlesToPlay, Location);
	}
}

void AShooterCharacter::PlayCascadeParticles(UParticleSystem* ParticlesToPlay, FTransform Location)
{
	if (ParticlesToPlay)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParticlesToPlay, Location);
	}
}

void AShooterCharacter::SendBullet()
{
	const USkeletalMeshSocket* BarrelSocket = EquippedWeapon->GetItemMesh()->GetSocketByName("BarrelSocket");

	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(EquippedWeapon->GetItemMesh());

		PlayCascadeParticles(MuzzleFlash, SocketTransform);
		
		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if (bBeamEnd)
		{
			PlayCascadeParticles(ImpactParticles, BeamEnd);

			if (BeamParticles)
			{
				UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
				if (Beam)
				{
					Beam->SetVectorParameter(FName("Target"), BeamEnd);
				}
			}
		}
	}
}

void AShooterCharacter::AimingButtonPressed()
{
	bAimingButtonPressed = true;

	if (CombatState != ECombatState::ECS_Reloading)	Aim();
}

void AShooterCharacter::AimingButtonReleased()
{
	bAimingButtonPressed = false;

	StopAiming();
}

void AShooterCharacter::ReloadButtonPressed()
{
	ReloadWeapon();
}

void AShooterCharacter::ReloadWeapon()
{
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	if (IsCarryingAmmo() && EquippedWeapon->CanReload())
	{
		CombatState = ECombatState::ECS_Reloading;

		if (bAiming) StopAiming();

		PlayMontageSection(ReloadMontage, EquippedWeapon->GetReloadSectionName());
	}
}

void AShooterCharacter::FinishReloading()
{
	CombatState = ECombatState::ECS_Unoccupied;

	if (EquippedWeapon == nullptr) return;

	const auto AmmoType{ EquippedWeapon->GetAmmoType() };

	if (IsCarryingAmmo())
	{
		int32 CarriedAmmoAmount = AmmoMap[AmmoType];
		const int32 MagEmptySpace = EquippedWeapon->GetMagazingCapacity() - EquippedWeapon->GetAmmoAmount();

		const int32 AmmoToAdd = MagEmptySpace > CarriedAmmoAmount ? CarriedAmmoAmount : MagEmptySpace;
		EquippedWeapon->ReloadAmmo(AmmoToAdd);
		AmmoMap[AmmoType] = AmmoMap[AmmoType] - AmmoToAdd;
	}

	if (bAimingButtonPressed) Aim();

}

FInterpLocation AShooterCharacter::GetInterpLocation(int32 Index)
{
	if (Index < InterpLocations.Num())
	{
		return InterpLocations[Index];
	}
	return FInterpLocation();
}

/*
* Returns false if the PC does not have a weapon equipped
* Returns false if the PC has 0 or no entry of the kind of ammo that the Equipped Weapon takes
* Returns true otherwise
*/
bool AShooterCharacter::IsCarryingAmmo()
{
	if (EquippedWeapon == nullptr) return false;

	auto AmmoType = EquippedWeapon->GetAmmoType();

	if (AmmoMap.Contains(AmmoType))
	{
		return AmmoMap[AmmoType] > 0;
	}
	return false;
}

/* Functions for reloading & moving the clip around */
void AShooterCharacter::GrabClip()
{
	if (EquippedWeapon == nullptr || HandSceneComponent == nullptr) return;

	ClipTransform = EquippedWeapon->GetClipBoneTransform();

	FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepRelative, true);
	HandSceneComponent->AttachToComponent(GetMesh(), AttachmentRules, FName("Hand_L"));
	HandSceneComponent->SetWorldTransform(ClipTransform);

	EquippedWeapon->SetClipIsMoving(true);
}

void AShooterCharacter::ReleaseClip()
{
	if (EquippedWeapon == nullptr) return;



	EquippedWeapon->SetClipIsMoving(false);
}

int32 AShooterCharacter::GetInterpLocationIndex()
{
	int32 LowestIndex = 1;
	int32 LowestCount = INT_MAX;

	for (int32 i = 1; i < InterpLocations.Num(); i++)
	{
		if (InterpLocations[i].ItemCount < LowestCount)
		{
			LowestIndex = i;
			LowestCount = InterpLocations[i].ItemCount;
		}
	}

	return LowestIndex;
}

void AShooterCharacter::UseInterpLocation(int32 Index)
{
	InterpLocations[Index].ItemCount += 1;
}

void AShooterCharacter::FreeInterpLocation(int32 Index)
{
	if (InterpLocations[Index].ItemCount > 0) InterpLocations[Index].ItemCount -= 1;
	else UE_LOG(LogTemp, Warning, TEXT("AShooterCharacter::FreeInterpLocation() Tried to free index %i that was already at 0"), Index);
}

