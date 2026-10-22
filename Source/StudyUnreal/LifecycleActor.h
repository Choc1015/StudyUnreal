// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MyItem.h"
#include "LifecycleActor.generated.h"

UCLASS()
class STUDYUNREAL_API ALifecycleActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALifecycleActor();
	virtual void PostInitializeComponents() override;

	UPROPERTY()
	int32 num = 0;

	UPROPERTY()
	UMyItem* ItemA;

	UMyItem* ItemB;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
