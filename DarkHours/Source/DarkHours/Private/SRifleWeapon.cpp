// Fill out your copyright notice in the Description page of Project Settings.

#include "SRifleWeapon.h"

// Sets default values
ASRifleWeapon::ASRifleWeapon()
{
	// Initialize components and variables
	// Variables
	ClipSize = 30; // Using the most generic clip size for assault rifles, clip size is customizable within BPs
	MaxAmmo = 90;

}

// Called when the game starts or when spawned
void ASRifleWeapon::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ASRifleWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}
