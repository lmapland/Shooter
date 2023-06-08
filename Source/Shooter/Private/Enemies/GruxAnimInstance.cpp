// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/GruxAnimInstance.h"
#include "Enemies/Enemy.h"

void UGruxAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	if (!Enemy)
	{
		Enemy = Cast<AEnemy>(TryGetPawnOwner());
		//UE_LOG(LogTemp, Warning, TEXT("Trying to get Grux AEnemy"));
	}
	else
	{
		FVector Velocity = Enemy->GetVelocity();
		Velocity.Z = 0.f;
		Speed = Velocity.Size();
	}
}
