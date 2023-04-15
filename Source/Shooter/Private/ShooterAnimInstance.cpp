// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (ShooterCharacter == nullptr)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}
	else
	{
		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f) bIsAccelerating = true;
		else bIsAccelerating = false;
		bAiming = ShooterCharacter->GetAiming();

		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		LastMovementOffsetYaw = ShooterCharacter->GetVelocity().Size() == 0 ? LastMovementOffsetYaw : MovementOffsetYaw;
	}
	TurnInPlace();
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;
	if (Speed > 0)
	{
		RootYawOffset = 0;
		CurrentCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		PreviousCharacterYaw = CurrentCharacterYaw;
		PreviousRotationCurve = 0.f;
		CurrentRotationCurve = 0.f;
		return;
	}

	PreviousCharacterYaw = CurrentCharacterYaw;
	CurrentCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
	const float YawDelta{ CurrentCharacterYaw - PreviousCharacterYaw };

	RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

	// 1.0 if turning, 0.0 if not
	const float Turning{ GetCurveValue(TEXT("Turning")) };
	if (Turning > 0)
	{
		PreviousRotationCurve = CurrentRotationCurve;
		CurrentRotationCurve = GetCurveValue(TEXT("Rotation"));
		const float DeltaRotation = CurrentRotationCurve - PreviousRotationCurve;

		// RootYawOffset > 0: turning left. Otherwise currently turning right
		RootYawOffset > 0 ? RootYawOffset -= DeltaRotation : RootYawOffset += DeltaRotation;

		const float ABSRootYawOffset{ FMath::Abs(RootYawOffset) };
		if (ABSRootYawOffset > 90.f)
		{
			const float YawExcess{ ABSRootYawOffset - 90.f };
			RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
		}
	}
}
