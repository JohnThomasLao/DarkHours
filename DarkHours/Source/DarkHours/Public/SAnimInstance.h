// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "SAnimInstance.generated.h"

class ASCharacter;

UCLASS()
class DARKHOURS_API USAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// Holds character movements speed
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float MovementSpeed;

	// Movement values - derive from owner character 
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Anim_InputX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Anim_InputY;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float Anim_TurnX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float MovementDirection;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsSprinting;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsFalling;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsAiming;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		bool bIsCrouching;

	// Holds the state at whether character holds initial direction or not
	UPROPERTY(BlueprintReadWrite)
		bool bReceivedInitialDirection;

	// Animation play rate - used for manipulating the anim play speed
	UPROPERTY(BlueprintReadWrite)
		float AnimPlayRate;

	// Ref to owner charater
	UPROPERTY(BlueprintReadWrite)
		ASCharacter* OwnerCharacter;

	// Leaning rotation of the character when turning
	UPROPERTY(BlueprintReadWrite)
		FRotator ProcedualLeaningRotation;

	// Character procedual leaning scale - the closer to 0, the more the character would lean
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
		float LeaningScale;

public:
	// Called for updating the character animation properties
	UFUNCTION(BlueprintCallable)
		void UpdateAnimationProperties(float DeltaTime);

	// Called for setting direction and receiving initial direction
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
		void SetDirectionAndReceiveInitialDirection();

};
