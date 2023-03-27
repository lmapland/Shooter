// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "ShooterCharacter.generated.h"

class USpringArmComponent;
class UInputComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class APlayerController;

UCLASS()
class SHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AShooterCharacter();
	virtual void Jump() override;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void Move(const FInputActionValue& Value); // handles forward, back, and side-to-side
	void Look(const FInputActionValue& Value);
	void ZoomIn();
	void ZoomOut();

	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* CharMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction; // mousemove

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* InteractAction; // e

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction; // spacebar

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* DodgeAction; // z

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* AttackAction; // click

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* EquipAction; // r

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomInAction; // mouse wheel scroll up
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* ZoomOutAction; // mouse wheel scroll up

	//UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	//UInputAction* SprintAction; // left shift

	
	/* Zooming */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MinZoomLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MaxZoomLength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float ZoomInOutAmount;


private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY()
	APlayerController* PlayerController;

};
