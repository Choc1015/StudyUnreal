// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TargetDummy.h"
#include "Turret.generated.h"

UCLASS()
class STUDYUNREAL_API ATurret : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ATurret();

	UPROPERTY()
	TWeakObjectPtr<ATargetDummy> FindObject = nullptr;

	UPROPERTY()
	ATargetDummy* RawTarget;

	UPROPERTY()
	float TimeCheck = 0;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
