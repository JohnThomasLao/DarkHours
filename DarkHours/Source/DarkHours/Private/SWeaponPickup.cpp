// Fill out your copyright notice in the Description page of Project Settings.

#include "SWeaponPickup.h"
#include "SWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASWeaponPickup::ASWeaponPickup()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Inititalize components and variables
	/* Components */
	PickupComp = CreateDefaultSubobject<UBoxComponent>(TEXT("PickupComponent"));
	RootComponent = PickupComp;
	PickupComp->InitBoxExtent(FVector(100.f, 100.f, 200.f));
	PickupComp->SetCollisionResponseToAllChannels(ECR_Overlap);

	WeaponRepMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponRepMeshComponent"));
	WeaponRepMeshComp->SetupAttachment(RootComponent);
	WeaponRepMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponRepMeshComp->SetCollisionResponseToAllChannels(ECR_Overlap);
	WeaponRepMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

}

// Called when the game starts or when spawned
void ASWeaponPickup::BeginPlay()
{
	Super::BeginPlay();

	WeaponRepMeshComp->SetSimulatePhysics(true); // Stimulate physics

	if (PendingPickupWeaponClass != NULL) {
		ASWeapon* PendingPickupWeapon = Cast<ASWeapon>(PendingPickupWeaponClass->GetDefaultObject());

		if (PendingPickupWeapon != NULL) {
			UpdateAmmo = PendingPickupWeapon->UpdateAmmo;
			ClipSize = PendingPickupWeapon->ClipSize;
			MaxAmmo = PendingPickupWeapon->MaxAmmo;
		}
	}
	
}

// Called every frame
void ASWeaponPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Returns weapon representable mesh component
UStaticMeshComponent* ASWeaponPickup::GetMeshComponent()
{
	return WeaponRepMeshComp;

}

