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
	
	if (ShooterCharacter)
	{
		bCrouching = ShooterCharacter->GetCrouching();
		bIsReloading = ShooterCharacter->GetCombatState() == ECombatState::ECS_Reloading;
		bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
		bAiming = ShooterCharacter->GetAiming();

		if (ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f) bIsAccelerating = true;
		else bIsAccelerating = false;

		FVector Velocity{ ShooterCharacter->GetVelocity() };
		Velocity.Z = 0;
		Speed = Velocity.Size();
		FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
		FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
		MovementOffsetYaw = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
		LastMovementOffsetYaw = ShooterCharacter->GetVelocity().Size() == 0 ? LastMovementOffsetYaw : MovementOffsetYaw;

		if (bIsReloading) OffsetState = EOffsetState::EOS_Reloading;
		else if (bIsInAir) OffsetState = EOffsetState::EOS_InAir;
		else if (bAiming) OffsetState = EOffsetState::EOS_Aiming;
		else OffsetState = EOffsetState::EOS_Hip;
	}
	TurnInPlace();
	Lean(DeltaTime);
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
	if (ShooterCharacter == nullptr) return;

	Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;

	if (Speed > 0 || bIsInAir)
	{
		RootYawOffset = 0;
		TIPCurrentCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
		TIPPreviousCharacterYaw = TIPCurrentCharacterYaw;
		PreviousRotationCurve = 0.f;
		CurrentRotationCurve = 0.f;
	}
	else
	{
		TIPPreviousCharacterYaw = TIPCurrentCharacterYaw;
		TIPCurrentCharacterYaw = ShooterCharacter->GetActorRotation().Yaw;

		RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - (TIPCurrentCharacterYaw - TIPPreviousCharacterYaw));

		// 1.0 if turning, 0.0 if not
		const float Turning{ GetCurveValue(TEXT("Turning")) };
		if (Turning > 0)
		{
			RecoilWeight = 0.f;

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
		else
		{
			RecoilWeight = 0.5f;
		}
	}

	if (bIsReloading || bAiming) RecoilWeight = 1.f;
	else if (bCrouching) RecoilWeight = .1f;
}

void UShooterAnimInstance::Lean(float DeltaTime)
{
	if (ShooterCharacter == nullptr) return;

	PreviousCharacterRotation = CurrentCharacterRotation;
	CurrentCharacterRotation = ShooterCharacter->GetActorRotation();
	const FRotator Delta{ UKismetMathLibrary::NormalizedDeltaRotator(CurrentCharacterRotation, PreviousCharacterRotation) };

	// Divide by a small value (DeltaTime) in order to get a large number
	const float Interp = FMath::FInterpTo(YawDelta, Delta.Yaw / DeltaTime, DeltaTime, 6.f);

	YawDelta = FMath::Clamp(Interp, -90.f, 90.f);
}
