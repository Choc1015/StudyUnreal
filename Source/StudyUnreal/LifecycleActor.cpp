// Fill out your copyright notice in the Description page of Project Settings.


#include "LifecycleActor.h"

// Sets default values
ALifecycleActor::ALifecycleActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	UE_LOG(LogTemp, Warning, TEXT("Constructor: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));
}

void ALifecycleActor::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	UE_LOG(LogTemp, Warning, TEXT("PostInitializeComponents: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));
}

// Called when the game starts or when spawned
void ALifecycleActor::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay"));
	UE_LOG(LogTemp, Warning, TEXT("BeginPlay: %s, World: %s"), *GetName(), GetWorld() ? TEXT("yes") : TEXT("no"));

	ItemA = NewObject<UMyItem>(this, TEXT("ItemA"));
	WatchA = ItemA;
	ItemC = NewObject<UMyItem>(this, TEXT("ItemC"));


}

void ALifecycleActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	UE_LOG(LogTemp, Warning, TEXT("WatchA valid: %s"), WatchA.IsValid() ? TEXT("yes") : TEXT("no"));
	UE_LOG(LogTemp, Warning, TEXT("ItemC valid: %s"), ItemC.IsValid() ? TEXT("yes") : TEXT("no"));
}

// Called every frame
void ALifecycleActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	num += 1;


}

