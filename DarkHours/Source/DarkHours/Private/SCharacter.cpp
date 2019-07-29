// Fill out your copyright notice in the Description page of Project Settings.

#include "SCharacter.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShake.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SkeletalMeshComponent.h"
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

	// Variables
	AimFOV = 30.f;
	FOVInterpSpeed = 15.f;

}

// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Initialize player controller
	PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	// Initialize charater default FOV
	DefaultFOV = CameraComp->FieldOfView;

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
		SpringArmComp->SocketOffset = FVector(0.f, 50.f, 10.f); // Move camera a bit to the right
		SpringArmComp->bEnableCameraLag = false;
		SpringArmComp->bEnableCameraRotationLag = false;
		SpringArmComp->TargetArmLength = 250.f; // Bring camera a bit closer
	}
	else {
		// Reset all the spring arm component values that are altered when aiming
		SpringArmComp->SocketOffset = FVector::ZeroVector;
		SpringArmComp->bEnableCameraLag = true;
		SpringArmComp->bEnableCameraRotationLag = true;
		SpringArmComp->TargetArmLength = 300.f;
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

}

void ASCharacter::AimEnd()
{
	bIsAiming = false;

}

void ASCharacter::CameraX(float Value)
{
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
