// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/BulletHitInterface.h"
#include "Enemy.generated.h"

class UParticleSystem;
class USoundCue;
class UHealthBarComponent;
class UBehaviorTree;
class AEnemyController;
class USphereComponent;
class UBoxComponent;
class AShooterCharacter;

UCLASS()
class SHOOTER_API AEnemy : public ACharacter, public IBulletHitInterface
{
	GENERATED_BODY()

public:
	AEnemy();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BulletHit_Implementation(FHitResult HitResult) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const & DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintImplementableEvent)
	void ShowHitNumber(int32 Damage, bool bIsHeadShot);

protected:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void StoreHitNumber(UUserWidget* HitNumber, FVector Location);

	UFUNCTION()
	void RemoveHitNumber(UUserWidget* HitNumber);

	void UpdateHitNumbers();

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateHitNumberLocation(UUserWidget* HitNumber, FVector Position);

	UFUNCTION()
	void OnAggroSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCombatSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnCombatSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	UFUNCTION()
	void OnLeftWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	UFUNCTION()
	void OnRightWeaponOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	
	void DoDamage(AShooterCharacter* Victim);

	UFUNCTION(BlueprintCallable)
	void SetStunned(bool Stunned);
	
	UFUNCTION(BlueprintCallable)
	void SetInCombatRange(bool InRange);

	UFUNCTION(BlueprintCallable)
	void PlayAttackMontage(FName Section, float PlayRate);
	
	UFUNCTION(BlueprintCallable)
	void FinishAttack();

	UFUNCTION(BlueprintPure)
	FName GetAttackSectionName();

	UFUNCTION(BlueprintCallable)
	void SetLeftWeaponActive(bool Active);

	UFUNCTION(BlueprintCallable)
	void SetRightWeaponActive(bool Active);

	void ResetCanAttack();

	UFUNCTION(BlueprintCallable)
	void FinishDeath();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float Health = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float MaxHealth = 100.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float BaseDamage = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	FString HeadBone;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	USoundBase* ImpactSound;

	UPROPERTY(VisibleAnywhere)
	USphereComponent* AggroSphere;
	
	UPROPERTY(VisibleAnywhere)
	USphereComponent* CombatSphere;
	
	UPROPERTY(VisibleAnywhere)
	UHealthBarComponent* HealthBarWidget;

	FTimerHandle HealthBarTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float HealthBarDisplayTime = 4.f;

	FTimerHandle HitReactTimer;

	bool bCanHitReact = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float HitReactDelayMin = 0.75f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float HitReactDelayMax = 2.f;

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float DeathTime = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Montage, meta = (AllowedPrivateAccess = "true"))
	UAnimMontage* HitMontage;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Montage, meta = (AllowedPrivateAccess = "true"))
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Montage, meta = (AllowedPrivateAccess = "true"))
	UAnimMontage* DeathMontage;

	/* Used to keep track of the current Hit Numbers being displayed so their position can be updated as appropriate */
	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	TMap<UUserWidget*, FVector> HitNumbers;

	UPROPERTY(EditAnywhere, Category = Combat, meta = (AllowedPrivateAccess = "true"))
	float HitNumberLifetime = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Artificial Intelligence", meta = (AllowedPrivateAccess = "true"))
	UBehaviorTree* BehaviorTree;

	UPROPERTY(EditAnywhere, Category = "Artificial Intelligence", meta = (AllowedPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint;

	UPROPERTY(EditAnywhere, Category = "Artificial Intelligence", meta = (AllowedPrivateAccess = "true", MakeEditWidget = "true"))
	FVector PatrolPoint2;

	AEnemyController* EnemyController;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bStunned = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowedPrivateAccess = "true", MakeEditWidget = "true"))
	float StunChance = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowedPrivateAccess = "true", MakeEditWidget = "true"))
	bool bInAttackRange = false;

	UPROPERTY(VisibleAnywhere, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	bool bCanAttack = true;

	FTimerHandle AttackWaitTimer;

	UPROPERTY(EditAnywhere, Category = "Combat", meta = (AllowedPrivateAccess = "true", MakeEditWidget = "true"))
	float AttackWaitTime = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	TArray<FName> AttackSections;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName LeftWeaponSocket = FName("FX_Trail_L_01");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	FName RightWeaponSocket = FName("FX_Trail_R_01");

private:
	void ShowHealthBar();
	void HideHealthBar();
	void Die();
	void PlayHitMontage(FName Section, float PlayRate);
	void ResetHitReact();
	void DestroyEnemy();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* LeftWeaponCollision;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	UBoxComponent* RightWeaponCollision;

	bool bDead = false;

public:
	FORCEINLINE FString GetHeadBoneName() const { return HeadBone; }
	FORCEINLINE UBehaviorTree* GetBehaviorTree() const { return BehaviorTree; }
};
