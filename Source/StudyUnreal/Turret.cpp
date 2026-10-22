// Fill out your copyright notice in the Description page of Project Settings.


#include "Turret.h"
#include "Kismet/GameplayStatics.h" 

// Sets default values
ATurret::ATurret()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// Called when the game starts or when spawned
void ATurret::BeginPlay()
{
	Super::BeginPlay();

	AActor* Found = UGameplayStatics::GetActorOfClass(GetWorld(), ATargetDummy::StaticClass());
	FindObject = Cast<ATargetDummy>(Found);
	RawTarget = Cast<ATargetDummy>(Found);

}

// Called every frame
void ATurret::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TimeCheck += DeltaTime;
	if (TimeCheck >= 1)
	{
		if (FindObject.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("%f"),GetDistanceTo(Cast<AActor>(FindObject)));
			TimeCheck = 0;
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("Target lost"));
			TimeCheck = 0;	
		}

		UE_LOG(LogTemp, Warning, TEXT("Raw is null: %s"), RawTarget == nullptr ? TEXT("yes") : TEXT("no"));

	}
}

