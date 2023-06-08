// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HealthBarComponent.h"
#include "Components/ProgressBar.h"
#include "Widgets/EnemyHealthBarWidget.h"

void UHealthBarComponent::SetHealthPercent(float Percent)
{
	if (nullptr == HealthBarWidget)
	{

		HealthBarWidget = Cast<UEnemyHealthBarWidget>(GetUserWidgetObject());
	}

	if (HealthBarWidget && HealthBarWidget->HealthProgressBar)
	{
		if (Percent < 0.f) Percent = 0.f;
		if (Percent > 100.f) Percent = 100.f;

		HealthBarWidget->HealthProgressBar->SetPercent(Percent);
	}
}
