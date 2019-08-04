// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class ASWeaponPickup;
class USphereComponent;

UCLASS()
class DARKHOURS_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	// Weapon (skeletal) mesh component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
		USkeletalMeshComponent* WeaponMeshComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Ref to pickup class of this weapon when character never/no longer possesses this weapon
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Pickup")
		TSubclassOf<class ASWeaponPickup> WeaponPickupClass;

	// Update amount of ammo
	int32 UpdateAmmo;

	// Max amount of ammo in a single clip
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammunition")
		int32 ClipSize;

	// Max amount of ammo this weapon has
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Ammunition")
		int32 MaxAmmo;

};
