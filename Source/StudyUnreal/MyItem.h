// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MyItem.generated.h"

/**
 * 
 */
UCLASS()
class STUDYUNREAL_API UMyItem : public UObject
{
	GENERATED_BODY()
	
public:
	virtual void BeginDestroy() override;

};
