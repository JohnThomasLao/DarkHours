// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "SRifleWeapon.h"
#include "SWeapon.h"
#include "SWeaponPickup.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShake.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/GameEngine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Automatically possess this character
	AutoPossessPlayer = EAutoReceiveInput::Player0;

	// Disable all the controller-rotation-based character rotations 
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character settings
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character will move toward camera pointing direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 250.f, 0.f);
	GetCharacterMovement()->JumpZVelocity = 300.f;
	GetCharacterMovement()->AirControl = 0.2f;
	GetCharacterMovement()->GetNavAgentPropertiesRef().bCanCrouch = true; // Crouching ability is somehow disabled by default
	GetCharacterMovement()->MaxWalkSpeed = 820.f;
	GetCharacterMovement()->MaxWalkSpeedCrouched = 740.f;
	GetCharacterMovement()->MaxAcceleration = 800.f;
	GetCharacterMovement()->BrakingFriction = 0.2f;

	// Initialize components and variables
	// Components
	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 300.f; // Determine the distance between the camera and the character
	SpringArmComp->bEnableCameraLag = true;
	SpringArmComp->CameraLagSpeed = 8.f;
	SpringArmComp->bEnableCameraRotationLag = true;
	SpringArmComp->CameraRotationLagSpeed = 10.f;
	SpringArmComp->bUsePawnControlRotation = true;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName); // Attach camera component to the 'tail' of the spring arm component
	CameraComp->bUsePawnControlRotation = false;

	GetCapsuleComponent()->SetGenerateOverlapEvents(true);

	// Variables
	AimFOV = 20.f;
	FOVInterpSpeed = 15.f;

	PrimaryDrawSocketName = "PrimaryDrawSocket";
	PrimaryHolsterSocketName = "PrimaryHolsterSocket";

	PrimaryWeapon = NULL;
	SecondaryWeapon = NULL;

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize player controller
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	// Initialize charater default FOV
	DefaultFOV = CameraComp->FieldOfView;

	GetCapsuleComponent()->OnComponentBeginOverlap.AddDynamic(this, &ASCharacter::OnStartOverlappingActors);
	GetCapsuleComponent()->OnComponentEndOverlap.AddDynamic(this, &ASCharacter::OnEndOverlappingActors);

}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Debug the direction value
	if (GEngine != NULL) {
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Red, FString::Printf(TEXT("Movement Direction: %f"), MovementDirection));
	}

	// Calculate movement direction after every frame
	CalculateCharacterMovementDirection(InputX, InputY);

	// Make target FOV for when aiming
	float AimTargetFOV = FMath::FInterpTo(CameraComp->FieldOfView, bIsAiming ? AimFOV : DefaultFOV, DeltaTime, FOVInterpSpeed);

	CameraComp->SetFieldOfView(AimTargetFOV);

	if (bIsAiming) {
		SpringArmComp->SocketOffset = FVector(0.f, 80.f, 15.f); // Move camera a bit to the right
		SpringArmComp->bEnableCameraLag = false;
		SpringArmComp->bEnableCameraRotationLag = false;
	}
	else {
		// Reset all the spring arm component values that are altered when aiming
		SpringArmComp->SocketOffset = FVector::ZeroVector;
		SpringArmComp->bEnableCameraLag = true;
		SpringArmComp->bEnableCameraRotationLag = true;
	}

	if (OverlappedWeaponPickup != NULL) {
		UKismetSystemLibrary::PrintString(this, UKismetSystemLibrary::GetDisplayName(OverlappedWeaponPickup), true, false, FColor::Green, 0.f);
	}

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setup in-game input bindings
	// Character input
	PlayerInputComponent->BindAxis("MoveX", this, &ASCharacter::MoveX);
	PlayerInputComponent->BindAxis("MoveY", this, &ASCharacter::MoveY);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &ASCharacter::SprintStart);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &ASCharacter::SprintEnd);

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::ToggleCrouch);

	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ASCharacter::JumpStart);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ASCharacter::JumpEnd);

	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ASCharacter::AimStart);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ASCharacter::AimEnd);

	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &ASCharacter::Interact);

	// Camera input
	PlayerInputComponent->BindAxis("CameraX", this, &ASCharacter::CameraX);
	PlayerInputComponent->BindAxis("CameraY", this, &ASCharacter::CameraY);

}

void ASCharacter::MoveX(float Value)
{
	InputX = Value;

	// Debug the character forward movement input value
	if (GEngine != NULL) {
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Input X: %f"), InputX));
	}

	if (Controller != NULL) {
		if (bIsSprinting) {
			Value *= 2;
		}

		// Determine the forward rotation
		FRotator NewRotation = Controller->GetControlRotation();
		FRotator NewYawRotation = FRotator(0.f, NewRotation.Yaw, 0.f);

		// Make a forward vector direction to move the character 
		FVector Direction = FRotationMatrix(NewYawRotation).GetUnitAxis(EAxis::X);

		// Move character using the forward vector direction
		AddMovementInput(Direction, Value / 3);
	}

}

void ASCharacter::MoveY(float Value)
{
	InputY = Value;

	// Debug the character right movement input value 
	if (GEngine != NULL) {
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Blue, FString::Printf(TEXT("Input Y: %f"), InputY));
	}

	if (Controller != NULL) {
		if (bIsSprinting) {
			Value *= 2;
		}

		// Determine the right rotation
		FRotator NewRotation = Controller->GetControlRotation();
		FRotator NewYawRotation = FRotator(0.f, NewRotation.Yaw, 0.f);

		// Make a right vector direction to move the character 
		FVector Direction = FRotationMatrix(NewYawRotation).GetUnitAxis(EAxis::Y);

		// Move character using the right vector direction
		AddMovementInput(Direction, Value / 3);
	}

}

void ASCharacter::SprintStart()
{
	bIsSprinting = true;

	if (PlayerController != NULL && CamShake_Sprinting != NULL) {
		PlayerController->ClientPlayCameraShake(CamShake_Sprinting); // Start playing sprinting cam shake when sprinting
	}

}

void ASCharacter::SprintEnd()
{
	bIsSprinting = false;

	if (PlayerController != NULL && CamShake_Sprinting != NULL) {
		PlayerController->ClientStopCameraShake(CamShake_Sprinting); // Stop playing sprinting cam shake when stop sprinting
	}

}

void ASCharacter::ToggleCrouch()
{
	if (GetCharacterMovement()->IsCrouching()) { // If character is found already in crouching state
		UnCrouch();
	}
	else {
		Crouch();
	}

}

void ASCharacter::JumpStart()
{
	Jump();

}

void ASCharacter::JumpEnd()
{
	StopJumping();

}

void ASCharacter::AimStart()
{
	bIsAiming = true;

	bUseControllerRotationYaw = true; // Allow character to rotate in place

}

void ASCharacter::AimEnd()
{
	bIsAiming = false;

	bUseControllerRotationYaw = false; // When stop aiming, character is not allowed to rotate in place

}

/* This function is only called when the character overlaps with any actor that has collision component */
void ASCharacter::Interact()
{
	Interaction_PrimaryWeapon();

}

void ASCharacter::OnStartOverlappingActors(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult)
{
	if (OtherActor != NULL && OtherActor != this && OtherActor->GetClass()->IsChildOf(ASWeaponPickup::StaticClass())) {
		OverlappedWeaponPickup = Cast<ASWeaponPickup>(OtherActor);
	}

}

void ASCharacter::OnEndOverlappingActors(UPrimitiveComponent * OverlappedComponent, AActor * OtherActor, UPrimitiveComponent * OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor != NULL && OtherActor != this) {
		OverlappedWeaponPickup = NULL;
	}

}

void ASCharacter::CameraX(float Value)
{
	TurnX = Value;

	AddControllerYawInput(Value); // Control camera component horizontally

}

void ASCharacter::CameraY(float Value)
{
	AddControllerPitchInput(Value); // Control camera component vertically

}

void ASCharacter::CalculateCharacterMovementDirection(float InputX, float InputY)
{
	// Make input vector using character movement value
	FVector InputVector = UKismetMathLibrary::MakeVector(InputX, InputY * -1.f, 0.f);

	// Make rotation using the input vector
	FRotator InputRotation = UKismetMathLibrary::MakeRotFromX(InputVector);

	// Make rotation using rotation of the camera component and the capsule component
	FRotator ComponentRotation = UKismetMathLibrary::NormalizedDeltaRotator(CameraComp->GetComponentRotation(), GetCapsuleComponent()->GetComponentRotation());

	// Make direction rotation normalizing input rotation and component rotation 
	FRotator DirectionRotation = UKismetMathLibrary::NormalizedDeltaRotator(ComponentRotation, InputRotation);

	// Assign the direction using the yaw value of the direction rotation
	MovementDirection = DirectionRotation.Yaw;

}

void ASCharacter::Interaction_PrimaryWeapon()
{
	// Holds the weapon that this character las posessed - if valid (character had picked up and possessed one)
	ASWeapon* LastPossessedWeapon = NULL;

	// Holds the weapon pickup ref that the character last overlapped
	ASWeaponPickup * LastWeaponPickupOverlapped = NULL;

	// Drop location of the dropped weapon
	FVector DropLocation;

	FHitResult TraceHit;
	FCollisionQueryParams TraceInfos;
	TraceInfos.bTraceComplex = true;
	TraceInfos.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(TraceHit, GetActorLocation(), GetActorLocation() + GetActorForwardVector() * 80.f, ECC_WorldStatic, TraceInfos);

	if (TraceHit.bBlockingHit) {
		DropLocation = TraceHit.ImpactPoint + (TraceHit.ImpactNormal * 5.f) + FVector(0.f, 20.f, 80.f);
	}
	else {
		DropLocation = TraceHit.TraceEnd + FVector(0.f, 20.f, 80.f);
	}

	// Spawn informations and parameters
	FActorSpawnParameters SpawnInfos;
	SpawnInfos.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn; // Always spawn, but ignore collision

	if (OverlappedWeaponPickup != NULL) {
		LastWeaponPickupOverlapped = OverlappedWeaponPickup; // Assign ref using the last overlapped weapon pickup

		if (PrimaryWeapon != NULL) {
			LastPossessedWeapon = Cast<ASWeapon>(PrimaryWeapon); // Assign ref using the last possess primary weapon - if valid

			PrimaryWeapon->Destroy(); // Destroy the last primary weapon on character holding socket - to drop it

			// Spawn the new primary weapon from weapon class of the overlapped weapon pickup
			PrimaryWeapon = GetWorld()->SpawnActor<ASRifleWeapon>(OverlappedWeaponPickup->PendingPickupWeaponClass, GetMesh()->GetSocketTransform(PrimaryHolsterSocketName), SpawnInfos);

			// Attach the new primary weapon to the character holster socket
			PrimaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryHolsterSocketName);

			if (LastPossessedWeapon->WeaponPickupClass != NULL) {
				// Spawn the pickup actor of the last possessed weapon
				ASWeaponPickup* WeaponPickup = GetWorld()->SpawnActor<ASWeaponPickup>(LastPossessedWeapon->WeaponPickupClass, DropLocation, FRotator::ZeroRotator, SpawnInfos);

				if (WeaponPickup != NULL) {
					UStaticMeshComponent* WeaponPickupMeshComp = WeaponPickup->GetMeshComponent(); // Access the mesh the pickup actor of the last weapon

					if (WeaponPickupMeshComp != NULL) {
						WeaponPickupMeshComp->AddTorqueInRadians(FVector(1.f) * 4000.f); // Flip when dropped using torque force
					}
				}
			}

			LastWeaponPickupOverlapped->Destroy(); // Destroy the last overlapped weapon pickup, as the character chose to pickup the new weapon
		}
		else { // When character has no primary weapon in possession
			if (OverlappedWeaponPickup->PendingPickupWeaponClass != NULL) {
				// Spawn primary weapon using weapon class from overlapped weapon pickup
				PrimaryWeapon = GetWorld()->SpawnActor<ASRifleWeapon>(OverlappedWeaponPickup->PendingPickupWeaponClass, GetMesh()->GetSocketTransform(PrimaryHolsterSocketName), SpawnInfos);

				// Attach the spawned primary weapon to holster socket
				PrimaryWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetNotIncludingScale, PrimaryHolsterSocketName);

				OverlappedWeaponPickup->Destroy(); // Destroy the overlapped weapon pickup, as the character chose to pickup weapon
			}
		}
	}

}
