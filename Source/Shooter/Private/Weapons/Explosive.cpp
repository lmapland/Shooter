// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Explosive.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AExplosive::BeginPlay()
{
	Super::BeginPlay();

}

void AExplosive::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AExplosive::BulletHit_Implementation(FHitResult HitResult)
{
	if (ExplodeSound)
	{
		//GetMesh()->GetBoneLocation(HitResult.BoneName)
		UGameplayStatics::PlaySoundAtLocation(this, ExplodeSound, GetActorLocation());
	}
	if (ExplodeParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ExplodeParticles, HitResult.Location, FRotator(0.f), true);
	}

	// TODO Apply explosive damage

	Destroy();
}

