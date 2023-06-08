// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemies/Enemy.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Widgets/HealthBarComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Enemies/EnemyController.h"
#include "ShooterCharacter.h"

AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarWidget->SetupAttachment(GetRootComponent());

	AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AggroSphere"));
	AggroSphere->SetupAttachment(GetRootComponent());

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());

	LeftWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftWeaponCollision"));
	LeftWeaponCollision->SetupAttachment(GetMesh(), FName("LeftWeaponBone"));

	RightWeaponCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("RightWeaponCollision"));
	RightWeaponCollision->SetupAttachment(GetMesh(), FName("RightWeaponBone"));

}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnAggroSphereOverlap);
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnCombatSphereOverlap);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnCombatSphereEndOverlap);
	LeftWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnLeftWeaponOverlap);
	RightWeaponCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnRightWeaponOverlap);
	
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LeftWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	LeftWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	LeftWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RightWeaponCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	RightWeaponCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	RightWeaponCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	HealthBarWidget->SetHealthPercent(Health / MaxHealth);
	HideHealthBar();

	EnemyController = Cast<AEnemyController>(GetController());
	if (EnemyController)
	{
		FVector WorldPatrolPoint = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(FName("PatrolPoint"), WorldPatrolPoint);

		FVector WorldPatrolPoint2 = UKismetMathLibrary::TransformLocation(GetActorTransform(), PatrolPoint2);
		EnemyController->GetBlackboardComponent()->SetValueAsVector(FName("PatrolPoint2"), WorldPatrolPoint2);

		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);

		EnemyController->RunBehaviorTree(BehaviorTree);
	}
}

/* I'm not a fan of the Enemy owning all this info about HitNumberWidgets.
 * It seems very controlling */
void AEnemy::StoreHitNumber(UUserWidget* HitNumber, FVector Location)
{
	HitNumbers.Add(HitNumber, Location);

	FTimerHandle HitNumberTimer;
	FTimerDelegate HitNumberDelegate;
	HitNumberDelegate.BindUFunction(this, FName("RemoveHitNumber"), HitNumber);
	GetWorldTimerManager().SetTimer(HitNumberTimer, HitNumberDelegate, HitNumberLifetime, false);
}

/* These hit number widgets are created in blueprints and I'm not a big
 * fan of creating them in bp and destroying them in c++.
 * Maybe I'm overreacting but it seems like there's no concrete "ownership"
 * and it may make the code more confusing in the future. */
void AEnemy::RemoveHitNumber(UUserWidget* HitNumber)
{
	HitNumbers.Remove(HitNumber);
	HitNumber->RemoveFromParent();
}

void AEnemy::UpdateHitNumbers()
{
	for (auto& HitPair : HitNumbers)
	{
		UpdateHitNumberLocation(HitPair.Key, HitPair.Value);
	}
}

void AEnemy::OnAggroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	AShooterCharacter* Character= Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(TEXT("Target"), Character);
	}
}

void AEnemy::OnCombatSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor) return;

	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		SetInCombatRange(true);
	}
}

void AEnemy::OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!OtherActor) return;

	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		SetInCombatRange(false);
	}
}

void AEnemy::OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		Character->PlayMeleeHitParticles(GetMesh()->GetSocketLocation(LeftWeaponSocket));
	}
}

void AEnemy::OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AShooterCharacter* Character = Cast<AShooterCharacter>(OtherActor);
	if (Character)
	{
		DoDamage(Character);
		Character->PlayMeleeHitParticles(GetMesh()->GetSocketLocation(RightWeaponSocket));
	}
}

void AEnemy::DoDamage(AShooterCharacter* Victim)
{
	if (!Victim) return;

	UGameplayStatics::ApplyDamage(Victim, BaseDamage, EnemyController, this, UDamageType::StaticClass());
}

void AEnemy::SetStunned(bool Stunned)
{
	//if (Stunned) { UE_LOG(LogTemp, Warning, TEXT("SetStunned: true")); }
	//else { UE_LOG(LogTemp, Warning, TEXT("SetStunned: false")); }

	bStunned = Stunned;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("IsStunned"), bStunned);
	}
}

void AEnemy::SetInCombatRange(bool InRange)
{
	bInAttackRange = InRange;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(TEXT("InAttackRange"), bInAttackRange);
	}
}

void AEnemy::FinishAttack()
{
	//bCanAttack = true;
}

FName AEnemy::GetAttackSectionName()
{
	if (AttackSections.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("There are no attack section names. Returning an empty FName."));
		return FName("");
	}

	const int32 Section = FMath::FRandRange(0, AttackSections.Num() - 1);
	return AttackSections[Section];
}

void AEnemy::SetLeftWeaponActive(bool Active)
{
	if (Active) LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	else LeftWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::SetRightWeaponActive(bool Active)
{
	if (Active) RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	else RightWeaponCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::ResetCanAttack()
{
	//UE_LOG(LogTemp, Warning, TEXT("ResetCanAttack: bCanAttack = true"));
	bCanAttack = true;
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), true);
	}
}

void AEnemy::FinishDeath()
{
	GetMesh()->bPauseAnims = true;
	UE_LOG(LogTemp, Warning, TEXT("FinishDeath(): pausing anims"));

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::DestroyEnemy, DeathTime);
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateHitNumbers();
}

void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::BulletHit_Implementation(FHitResult HitResult)
{
	if (ImpactSound)
	{
		//GetMesh()->GetBoneLocation(HitResult.BoneName)
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, ImpactParticles, HitResult.Location, FRotator(0.f), true);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (bDead) return DamageAmount;

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsObject(FName("Target"), DamageCauser);
	}

	Health = FMath::Clamp(Health - DamageAmount, 0.f, MaxHealth);

	if (Health == 0.f)
	{
		Die();
	}
	else
	{
		HealthBarWidget->SetHealthPercent(Health / MaxHealth);
		ShowHealthBar();
		GetWorldTimerManager().SetTimer(HealthBarTimer, this, &AEnemy::HideHealthBar, HealthBarDisplayTime);

		const float Stunned = FMath::FRandRange(0.f, 1.f);
		if (Stunned <= StunChance)
		{
			SetStunned(true);
			PlayHitMontage(FName("FromFront"), 1.f);
		}
	}

	return DamageAmount;
}

void AEnemy::ShowHealthBar()
{
	HealthBarWidget->SetVisibility(true);
}

void AEnemy::HideHealthBar()
{
	HealthBarWidget->SetVisibility(false);
}

void AEnemy::Die()
{
	bDead = true;

	HideHealthBar();

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);
	}

	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("IsDead"), true);
		EnemyController->StopMovement();
	}
}

void AEnemy::PlayHitMontage(FName Section, float PlayRate)
{
	if (bCanHitReact)
	{
		bCanHitReact = false;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && HitMontage)
		{
			AnimInstance->Montage_Play(HitMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section);
		}
		const float HitReactTime = FMath::FRandRange(HitReactDelayMin, HitReactDelayMax);
		GetWorldTimerManager().SetTimer(HitReactTimer, this, &AEnemy::ResetHitReact, HitReactTime);
	}
}

void AEnemy::PlayAttackMontage(FName Section, float PlayRate)
{
	if (bInAttackRange)
	{
		bCanAttack = false;
		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		if (AnimInstance && AttackMontage)
		{
			AnimInstance->Montage_Play(AttackMontage, PlayRate);
			AnimInstance->Montage_JumpToSection(Section);
		}
	}
	GetWorldTimerManager().SetTimer(AttackWaitTimer, this, &AEnemy::ResetCanAttack, AttackWaitTime);
	if (EnemyController)
	{
		EnemyController->GetBlackboardComponent()->SetValueAsBool(FName("CanAttack"), false);
	}
}

void AEnemy::ResetHitReact()
{
	bCanHitReact = true;
}

void AEnemy::DestroyEnemy()
{
	Destroy();
}
