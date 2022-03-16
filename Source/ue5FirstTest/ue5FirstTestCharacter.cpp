// Copyright Epic Games, Inc. All Rights Reserved.

#include "ue5FirstTestCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "SHealthComponent.h"
#include "SWeapon.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// Aue5FirstTestCharacter

Aue5FirstTestCharacter::Aue5FirstTestCharacter() : ACharacter::ACharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;
	SetReplicateMovement(true);

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComponent"));

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
	ZoomedFOV = 62.0f;
	ZoomInterpSpeed = 20.0f;
	RightHandSocket = "hand_rGrip";

	bDied = false;
	bReplicates = true;
}

void Aue5FirstTestCharacter::BeginPlay()
{
	Super::BeginPlay();

	DefaultFOV = FollowCamera->FieldOfView;

	SetCurrentWeapon(StartWeaponClass);

	HealthComponent->OnHealthChanged.AddDynamic(this, &Aue5FirstTestCharacter::OnHealthChanged);

}


void Aue5FirstTestCharacter::SetCurrentWeapon(TSubclassOf<ASWeapon> weapon)
{

	if (CurrentWeapon) { CurrentWeapon->Destroy(); }

	if (GetLocalRole() == ROLE_Authority)
	{
		FActorSpawnParameters SpawnParams;

		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		CurrentWeapon = GetWorld()->SpawnActor<ASWeapon>(weapon, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (CurrentWeapon)
		{
			CurrentWeapon->SetOwner(this);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, RightHandSocket);
		}

	}

}

void Aue5FirstTestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	float TargetFov = bWantsToZoom ? ZoomedFOV : DefaultFOV;

	FollowCamera->SetFieldOfView(FMath::FInterpTo(FollowCamera->FieldOfView, TargetFov,GetWorld()->DeltaTimeSeconds, ZoomInterpSpeed));

}

//////////////////////////////////////////////////////////////////////////
// Input

void Aue5FirstTestCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &Aue5FirstTestCharacter::ServerBeginJump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &Aue5FirstTestCharacter::ServerEndJump);


	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &Aue5FirstTestCharacter::BeginSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &Aue5FirstTestCharacter::EndSprint);

	GetMovementComponent()->GetNavAgentPropertiesRef().bCanCrouch = true;

	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &Aue5FirstTestCharacter::BeginCrouch);
	PlayerInputComponent->BindAction("Crouch", IE_Released, this, &Aue5FirstTestCharacter::EndCrouch);


	PlayerInputComponent->BindAction("ADS", IE_Pressed, this, &Aue5FirstTestCharacter::BeginZoom);
	PlayerInputComponent->BindAction("ADS", IE_Released, this, &Aue5FirstTestCharacter::EndZoom);


	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &Aue5FirstTestCharacter::BeginFire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &Aue5FirstTestCharacter::EndFire);

	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &Aue5FirstTestCharacter::BeginReload);

	PlayerInputComponent->BindAction("SwitchWeapon", IE_Pressed, this, &Aue5FirstTestCharacter::BeginReload);

	PlayerInputComponent->BindAxis("MoveForward", this, &Aue5FirstTestCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &Aue5FirstTestCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &Aue5FirstTestCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &Aue5FirstTestCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &Aue5FirstTestCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &Aue5FirstTestCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &Aue5FirstTestCharacter::OnResetVR);
}

void Aue5FirstTestCharacter::BeginReload_Implementation()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->bCanReload())
		{
			bWantsToReload = true;
		}
	}
}

void Aue5FirstTestCharacter::EndReload_Implementation()
{
	if (CurrentWeapon)
	{
		bWantsToReload = false;
		CurrentWeapon->Reload();
	}
}

FVector Aue5FirstTestCharacter::GetPawnViewLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation();
	}
	return Super::GetPawnViewLocation();
}


void Aue5FirstTestCharacter::OnResetVR()
{
	// If ue5FirstTest is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in ue5FirstTest.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void Aue5FirstTestCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void Aue5FirstTestCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}



/* Jump Client and Server*/
void Aue5FirstTestCharacter::ServerBeginJump_Implementation()
{
	bJumping = true;
}

void Aue5FirstTestCharacter::ServerEndJump_Implementation()
{
	bJumping = false;
}



/* Crouch Client and Server*/
void Aue5FirstTestCharacter::BeginCrouch()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerBeginCrouch();
	}
	Crouch();
}

void Aue5FirstTestCharacter::EndCrouch()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerEndCrouch();
	}
	UnCrouch();
}

void Aue5FirstTestCharacter::BeginSprint()
{
	bSprint = true;
}

void Aue5FirstTestCharacter::EndSprint()
{
	bSprint = false;
}

void Aue5FirstTestCharacter::ServerBeginCrouch_Implementation()
{
		Crouch();
}

void Aue5FirstTestCharacter::ServerEndCrouch_Implementation()
{
		UnCrouch();
}



void Aue5FirstTestCharacter::BeginZoom_Implementation()
{
		bWantsToZoom = true;
}
//bool Aue5FirstTestCharacter::BeginZoom_Validate()
//{
//	return true;
//}


void Aue5FirstTestCharacter::EndZoom_Implementation()
{
		bWantsToZoom = false;
}
//bool Aue5FirstTestCharacter::EndZoom_Validate()
//{
//	return true;
//}


void Aue5FirstTestCharacter::BeginFire()
{
	if (bDied) { EndFire(); }

	if (CurrentWeapon)
	{
		if (CurrentWeapon->GetAmmo() > 0)
		{
			CurrentWeapon->StartFire();
		}
		else
		{
			BeginReload();
		}
	}
}

void Aue5FirstTestCharacter::EndFire()
{
	if (CurrentWeapon)
	{
		CurrentWeapon->StopFire();
	}
}


void Aue5FirstTestCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void Aue5FirstTestCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void Aue5FirstTestCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		if (bSprint)
		{
			AddMovementInput(Direction, Value);
		}
		else if(GetMovementComponent()->Velocity.Size() < 600.0f)
		{
			 AddMovementInput(Direction, Value); 
		}
	}
}

void Aue5FirstTestCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction

		if (bSprint)
		{
			AddMovementInput(Direction, Value);
		}
		else if (GetMovementComponent()->Velocity.Size() < 600.0f)
		{
			AddMovementInput(Direction, Value); 
		}
	}
}

void Aue5FirstTestCharacter::SwitchWeapon()
{
	if (CurrentWeapon)
	{
		if (CurrentWeapon->StaticClass() == WeaponSlot1Class)
		{
			SetCurrentWeapon(WeaponSlot2Class);
		}
		else
		{
			SetCurrentWeapon(WeaponSlot1Class);
		}
	}
}

void Aue5FirstTestCharacter::OnHealthChanged(USHealthComponent* HealthComp, float Health, float HealthDelta,
	const UDamageType* damageType, AController* Instegator, AActor* DamageCauser)
{
	if (Health <= 0.0f)
	{
		if (CurrentWeapon)
		{
			CurrentWeapon->StopFire();
		}
		bDied = true;

		GetMovementComponent()->StopMovementImmediately();
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		DetachFromControllerPendingDestroy();

		CurrentWeapon->Destroy();

		SetLifeSpan(10.0f);

	}
}

int Aue5FirstTestCharacter::GetAmmonInClip()
{
	return CurrentWeapon->GetAmmo();
}


void Aue5FirstTestCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(Aue5FirstTestCharacter, CurrentWeapon);
	DOREPLIFETIME(Aue5FirstTestCharacter, bDied);
	DOREPLIFETIME(Aue5FirstTestCharacter, bWantsToZoom);
	DOREPLIFETIME(Aue5FirstTestCharacter, bWantsToReload);
	DOREPLIFETIME(Aue5FirstTestCharacter, bJumping);
	DOREPLIFETIME(Aue5FirstTestCharacter, AimAngle);
}