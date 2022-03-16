// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GenericTeamAgentInterface.h"
#include "ue5FirstTestCharacter.generated.h"


class ASWeapon;
class USHealthComponent;

UCLASS(config=Game)
class Aue5FirstTestCharacter : public ACharacter, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USHealthComponent* HealthComponent;

public:
	Aue5FirstTestCharacter();

	virtual void Tick(float DeltaTime) override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;

	UFUNCTION( BlueprintCallable)
	bool bGetReload() { return bWantsToReload; }

	UFUNCTION(Server, reliable, BlueprintCallable)
	void EndReload();

	UFUNCTION(BlueprintCallable)
	void SetCurrentWeapon(TSubclassOf<ASWeapon> weapon);

	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	virtual FVector GetPawnViewLocation() const override;

	UFUNCTION(BlueprintCallable, Category = "Player")
		int GetAmmonInClip();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void BeginFire();

	UFUNCTION(BlueprintCallable, Category = "Player")
		void EndFire();

	UFUNCTION(Server, reliable, BlueprintCallable, Category = "Player")
		void BeginReload();

	UFUNCTION(Server, reliable, BlueprintCallable)
		void BeginZoom();

	UFUNCTION(Server, reliable, BlueprintCallable)
		void EndZoom();

	UFUNCTION(BlueprintCallable)
		bool bIsDied() { return bDied; }

	FORCEINLINE class USHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintCallable, Category = "Player")
	class ASWeapon* GetWeapon() const { return CurrentWeapon; }

protected:

	virtual void BeginPlay() override;

	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

	UFUNCTION(Server, reliable)
		void ServerBeginJump();

	UFUNCTION(Server, reliable)
	void ServerEndJump();

	UFUNCTION()
	void BeginCrouch();

	UFUNCTION()
	void EndCrouch();

	UFUNCTION()
	void BeginSprint();

	UFUNCTION()
	void EndSprint();

	UFUNCTION(Server, reliable)
	void ServerBeginCrouch();

	UFUNCTION(Server, reliable)
	void ServerEndCrouch();




	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	float AimAngle;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Player")
	bool bWantsToZoom;


	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bWantsToReload;


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	float ZoomedFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	float ZoomInterpSpeed;

	// Set during begin Play
	float DefaultFOV;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Player")
	TSubclassOf<ASWeapon> StartWeaponClass;


	UPROPERTY(Replicated, BlueprintReadOnly)
	ASWeapon* CurrentWeapon;

	
	UPROPERTY(VisibleDefaultsOnly, Category = "Player")
	FName RightHandSocket; 

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta, const UDamageType* damageType, AController* Instegator, AActor* DamageCauser);

	UPROPERTY(Replicated,BlueprintReadOnly, Category="Player")
	bool bDied;

	UPROPERTY(BlueprintReadOnly, Category = "Player")
	bool bSprint;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Player")
	bool bJumping;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimePrope) const override;

protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	UPROPERTY()
	FGenericTeamId TeamID;


};

