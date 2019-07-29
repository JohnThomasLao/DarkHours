// Fill out your copyright notice in the Description page of Project Settings.

#include "SAnimInstance.h"
#include "SCharacter.h"
#include "TimerManager.h"
#include "Engine/GameEngine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

void USAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
	// Get owner pawn
	APawn* OwnerPawn = TryGetPawnOwner();

	if (OwnerPawn != NULL) {
		// Update animation properties from owner pawn movement component
		bIsFalling = OwnerPawn->GetMovementComponent()->IsFalling();
		bIsCrouching = OwnerPawn->GetMovementComponent()->IsCrouching();

		// Cast owner pawn to owner character
		OwnerCharacter = Cast<ASCharacter>(OwnerPawn);

		if (OwnerCharacter != NULL) {
			// Update animation properties from owner character 
			bIsSprinting = OwnerCharacter->bIsSprinting;
			Anim_InputX = OwnerCharacter->InputX; // Set animation fwd / bwd
			Anim_InputY = OwnerCharacter->InputY; // Set animation rgt / lft

			if (bIsSprinting) {
				MovementSpeed = FMath::Clamp(UKismetMathLibrary::Abs(Anim_InputX) + UKismetMathLibrary::Abs(Anim_InputY), 0.f, 1.f) * 2.f; // Speed up twice

				AnimPlayRate = 1.45f; // Animation speed plays faster in sprinting state
			}
			else {
				MovementSpeed = FMath::Clamp(UKismetMathLibrary::Abs(Anim_InputX) + UKismetMathLibrary::Abs(Anim_InputY), 0.f, 1.f); // Speed stays the same

				AnimPlayRate = 1.f; // Animation speed is normal if not in sprinting state
			}

			// Debug character movement speed
			if (GEngine != NULL) {
				GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Purple, FString::Printf(TEXT("Movement Speed: %f"), MovementSpeed));
			}

			if (MovementSpeed > 0.01f) { // If speed > 0.01 then set direction input to anim
				if (bReceivedInitialDirection) {
					if (MovementSpeed < 0.01f) { // If speed < 0.01 then reset received init direction value for next event
						bReceivedInitialDirection = false;
					}
				}
				else {
					SetDirectionAndReceiveInitialDirection();
				}
			}
			else {
				if (MovementSpeed < 0.01f) { // If speed < 0.01 then reset received init direction value for next event
					bReceivedInitialDirection = false;
				}
			}

			if (!bIsFalling) {
				// Calculate character leaning rotation to be used when character turns
				float LeaningAxis = (UKismetMathLibrary::Dot_VectorVector(OwnerCharacter->GetActorRightVector(), OwnerCharacter->GetVelocity()) / LeaningScale) * OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed;

				ProcedualLeaningRotation = UKismetMathLibrary::MakeRotator(0.f, LeaningAxis, 0.f);
			}
		}
	}

}
