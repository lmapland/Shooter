// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Engine/DataTable.h"
#include "Footstep.generated.h"

class UNiagaraSystem;

USTRUCT(BlueprintType)
struct FFootstepSurfaceDataTable : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString Name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* SurfaceParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* SurfaceSound;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UNiagaraSystem* LandingParticles;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USoundBase* LandingSound;

};

/**
 * 
 */
UCLASS()
class SHOOTER_API UFootstep : public UAnimNotify
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable)
	void ExecuteFootstepEffects(FVector SocketLoc);
	
	UFUNCTION(BlueprintCallable)
	void ExecuteJumpLandEffects(FVector SocketLoc);

private:
	EPhysicalSurface GetSurfaceType(FVector SocketLoc);
	bool SetRecord(EPhysicalSurface CurrentHitSurface);
	void PlayFootstepEffects(FVector Loc);
	void PlayJumpLandEffects(FVector Loc);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	UDataTable* SurfaceDataTable;

	FFootstepSurfaceDataTable* SurfaceRecord;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DataTable", meta = (AllowPrivateAccess = "true"))
	float WaterLevel;

	TEnumAsByte<EPhysicalSurface> PreviousHitSurface;
};
