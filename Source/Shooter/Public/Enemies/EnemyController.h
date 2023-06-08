// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "EnemyController.generated.h"

class UBehaviorTreeComponent;

/**
 * 
 */
UCLASS()
class SHOOTER_API AEnemyController : public AAIController
{
	GENERATED_BODY()

public:
	AEnemyController();
	virtual void OnPossess(APawn* InPawn) override;

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Artificial Intelligence")
	UBlackboardComponent* BlackboardComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Artificial Intelligence")
	UBehaviorTreeComponent* BTComponent;

public:
	FORCEINLINE UBlackboardComponent* GetBlackboardComponent() const { return BlackboardComponent; }
};
