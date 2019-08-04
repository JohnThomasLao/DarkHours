// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeaponPickup.generated.h"

class ASWeapon;
class UBoxComponent;

UCLASS()
class DARKHOURS_API ASWeaponPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeaponPickup();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	/* Components */
	// Pickup component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UBoxComponent* PickupComp;

	// Weapon visual representing component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Components")
		UStaticMeshComponent* WeaponRepMeshComp;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Weapon to be possessed by character is this object is picked
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Weapon")
		TSubclassOf<ASWeapon> PendingPickupWeaponClass;

	// Return weapon pickup rep mesh component 
	UStaticMeshComponent* GetMeshComponent();

	// Update amount of ammo
	int32 UpdateAmmo;

	// Max amount of ammo in a single clip
	UPROPERTY(BlueprintReadWrite, Category = "Ammunition")
		int32 ClipSize;

	// Max amount of ammo the pending pickup weapon has
	UPROPERTY(BlueprintReadWrite, Category = "Ammunition")
		int32 MaxAmmo;

};
