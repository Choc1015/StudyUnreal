// Fill out your copyright notice in the Description page of Project Settings.


#include "MyItem.h"

void UMyItem::BeginDestroy()
{
	Super::BeginDestroy();

	UE_LOG(LogTemp, Warning, TEXT("BeginDestroy: %s"), *GetName());
}
