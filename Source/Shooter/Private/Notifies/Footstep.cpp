// Fill out your copyright notice in the Description page of Project Settings.


#include "Notifies/Footstep.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "../Shooter.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"


void UFootstep::ExecuteFootstepEffects(FVector SocketLoc)
{
	TEnumAsByte<EPhysicalSurface> CurrentHitSurface = GetSurfaceType(SocketLoc);

	if (SetRecord(CurrentHitSurface.GetValue())) PlayFootstepEffects(SocketLoc);
}

void UFootstep::ExecuteJumpLandEffects(FVector SocketLoc)
{
	TEnumAsByte<EPhysicalSurface> CurrentHitSurface = GetSurfaceType(SocketLoc);

	if (SetRecord(CurrentHitSurface.GetValue())) PlayJumpLandEffects(SocketLoc);
}

EPhysicalSurface UFootstep::GetSurfaceType(FVector SocketLoc)
{
	if (SocketLoc.Z < WaterLevel)
	{
		return EPS_Water;
	}

	FHitResult HitResult;
	FVector Start = SocketLoc + FVector(0.f, 0.f, 25.f);
	FVector End = Start + FVector(0.f, 0.f, -400.f);
	FCollisionQueryParams QueryParams;
	QueryParams.bReturnPhysicalMaterial = true;

	GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECollisionChannel::ECC_Visibility, QueryParams);

	return HitResult.bBlockingHit ? HitResult.PhysMaterial->SurfaceType : EPhysicalSurface::SurfaceType_Default;
}

bool UFootstep::SetRecord(EPhysicalSurface CurrentHitSurface)
{
	if (SurfaceDataTable)
	{
		if (PreviousHitSurface != CurrentHitSurface || !SurfaceRecord)
		{
			UE_LOG(LogTemp, Warning, TEXT("Getting new data row"));
			SurfaceRecord = SurfaceDataTable->FindRow<FFootstepSurfaceDataTable>(UEnum::GetValueAsName(CurrentHitSurface), FString("GetSurfaceTypeFootsteps"));

			if (!SurfaceRecord) return false;
		}
	}
	else return false;

	PreviousHitSurface = CurrentHitSurface;

	return true;
}

void UFootstep::PlayFootstepEffects(FVector Loc)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceRecord->SurfaceSound, Loc);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SurfaceRecord->SurfaceParticles, Loc);
}

void UFootstep::PlayJumpLandEffects(FVector Loc)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), SurfaceRecord->LandingSound, Loc);

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SurfaceRecord->LandingParticles, Loc);
}

