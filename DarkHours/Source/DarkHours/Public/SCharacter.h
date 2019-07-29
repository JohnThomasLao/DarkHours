// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SCharacter.generated.h"

class UCameraComponent;
class UCameraShake;
class USpringArmComponent;


UCLASS()
class DARKHOURS_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Components
	// Spring arm component to control the camera component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
		USpringArmComponent* SpringArmComp;

	// Camera component
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Components")
		UCameraComponent* CameraComp;

	// Binding functions
	// Character input
	void MoveX(float Value);
	void MoveY(float Value);

	void SprintStart();
	void SprintEnd();

	void ToggleCrouch();

	void JumpStart();
	void JumpEnd();

	void AimStart();
	void AimEnd();

	// Camera input
	void CameraX(float Value);
	void CameraY(float Value);

	// Calculates character movement direction
	void CalculateCharacterMovementDirection(float InputX, float InputY);

	// Variables
	// Ref to sprinting camera shake
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		TSubclassOf<UCameraShake> CamShake_Sprinting;

	// Ref to firing camera shake
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Camera)
		TSubclassOf<UCameraShake> CamShake_FiringWeapon;

	// Player controller - using ref of this character
	APlayerController* PlayerController;

	// FOV when character is aiming
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
		float AimFOV;

	// Character default FOV
	float DefaultFOV;

	// FOV transition interpolation speed
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Camera)
		float FOVInterpSpeed;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Variables
	// Character movement values
	UPROPERTY(BlueprintReadWrite)
		float InputX;

	UPROPERTY(BlueprintReadWrite)
		float InputY;

	UPROPERTY(BlueprintReadWrite)
		float MovementDirection;

	bool bIsSprinting;

	bool bIsAiming;

};
