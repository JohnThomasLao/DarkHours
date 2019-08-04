// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SWeapon.h"
#include "SRifleWeapon.generated.h"

UCLASS()
class DARKHOURS_API ASRifleWeapon : public ASWeapon
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASRifleWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
