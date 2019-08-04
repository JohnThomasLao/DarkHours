// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeapon.h"
#include "SWeaponPickup.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Initialize components and variables
	// Components
	WeaponMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMeshComponent"));
	RootComponent = WeaponMeshComp;

	// Variables
	ClipSize = 0;
	MaxAmmo = 0;

}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	// Fill the update amount of ammo with clip size
	UpdateAmmo = ClipSize;

}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

