// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "TimerManager.h"
#include "../ue5FirstTest.h"
#include "Net/UnrealNetwork.h"
//#include "NiagaraFunctionLibrary.h
//#include "Niagara/Public/NiagaraFunctionLibrary.h"
#include "D:/UnrealEngine-5.0.0-early-access-2/Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraFunctionLibrary.h"
#include "D:/UnrealEngine-5.0.0-early-access-2/Engine/Plugins/FX/Niagara/Source/Niagara/Public/NiagaraComponent.h"

static int32 DebugWeaponDrawing = 0;
FAutoConsoleVariableRef CVARDebugWeaponDrawing(
	TEXT("COOP.DebugWeapons"), 
	DebugWeaponDrawing, 
	TEXT("Draw Debug lines for Weapons"), 
	ECVF_Cheat
);

// Sets default values
ASWeapon::ASWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	RootComponent = MeshComp;

	MuzzleName = "Muzzle";
	TraceTargetName = "BeamEnd";

	BaseDamage = 20.0f;
	VitalDamageMultiplier = 2.0f;
	FireRate = 1.0f;
	NegativeAccuracy = 0.005f;
	AccuracyJump = 0.0f;

	bReplicates = true;

	NetUpdateFrequency = 66.0f;
	MinNetUpdateFrequency = 33.0f;
	Range = 6000.0f;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	TimeBetweenShots = 60 / FireRate;

	AmmoInClip = ClipSize;
}

void ASWeapon::Fire()
{
	//trace from pawn eyes to croos hair
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerFire();
	}


	AActor* WeaponOwner = GetOwner();
	if (WeaponOwner) 
	{
		if (AmmoInClip > 0)
		{
			FVector EyeLocation;
			FRotator EyeRotation;

			WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();

			AccuracyJump += NegativeAccuracy;

			UE_LOG(LogTemp, Warning, TEXT("direction before: %f, %f, %f"), ShotDirection.X, ShotDirection.Y, ShotDirection.Z);
			ShotDirection = FMath::VRandCone(ShotDirection,AccuracyJump, AccuracyJump);
			UE_LOG(LogTemp, Warning, TEXT("direction After: %f, %f, %f"), ShotDirection.X, ShotDirection.Y, ShotDirection.Z);
			//ShotDirection.Z += FMath::RandRange(-AccuracyJump, AccuracyJump);
			//ShotDirection.X += FMath::RandRange(-AccuracyJump, AccuracyJump);
			


			FVector TraceEnd = EyeLocation + (ShotDirection * Range);

			// Particle "Target" parameter
			FVector TraceEndPoint = TraceEnd;

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(WeaponOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;
			QueryParams.bReturnPhysicalMaterial = true;

			if (FireSound)
			{
				UGameplayStatics::SpawnSoundAtLocation(
					GetWorld(),
					FireSound,
					GetActorLocation(),
					FRotator::ZeroRotator
				);
			}

			EPhysicalSurface SurfaceType = SurfaceType_Default;

			FHitResult Hit;
			if (GetWorld()->LineTraceSingleByChannel(
				Hit,
				EyeLocation,
				TraceEnd,
				COLLISION_WEAPON,
				QueryParams
			))
			{
				TraceEndPoint = Hit.ImpactPoint;

				FVector normal = Hit.ImpactNormal;

				AActor* HitActor = Hit.GetActor();


				SurfaceType = UPhysicalMaterial::DetermineSurfaceType(Hit.PhysMaterial.Get());
				PlayImpactEffects(SurfaceType, TraceEndPoint, normal);


				float damage = 0.0f;
				if (SurfaceType == SURFACE_FLESHVITAL) { damage = BaseDamage * VitalDamageMultiplier; }
				else { damage = BaseDamage; }

				UGameplayStatics::ApplyPointDamage(
					HitActor,
					damage,
					ShotDirection,
					Hit,
					WeaponOwner->GetInstigatorController(),
					WeaponOwner,
					DamageType
				);

			}

			if (DebugWeaponDrawing > 0)
			{
				DrawDebugLine(
					GetWorld(),
					EyeLocation,
					TraceEnd,
					FColor::White,
					false,
					0.5f,
					0U,
					1.0f);
			}

			PlayFireEffects(TraceEndPoint);

			if (GetLocalRole() == ROLE_Authority)
			{
				HitScanTrace.TraceEnd = TraceEndPoint;
			}

			lastTimeFired = GetWorld()->TimeSeconds;
			AmmoInClip--;
		}
		else
		{
			if (EmptySound)
			{
				UGameplayStatics::SpawnSoundAtLocation(
					GetWorld(),
					EmptySound,
					GetActorLocation(),
					FRotator::ZeroRotator
				);
			}
		}
	}
	else
	{
		StopFire();
	}
}

void ASWeapon::OnRep_HitScanTrace()
{
	PlayFireEffects(HitScanTrace.TraceEnd);

	PlayImpactEffects(HitScanTrace.SurfaeType, HitScanTrace.TraceEnd, HitScanTrace.Direction);
}

void ASWeapon::ServerFire_Implementation()
{
	if (!GetOwner()) {
		StopFire();
	}
	else
	{
		Fire();
	}
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}


void ASWeapon::PlayImpactEffects(const EPhysicalSurface& SurfaceType, const FVector& TraceEndPoint, const FVector& Direction)
{

	USoundBase* SelectedSound = nullptr;
	UParticleSystem* SelectedParticles = nullptr;
	UNiagaraSystem* SelectedNiaParticles = nullptr;
	switch (SurfaceType)
	{
	case SURFACE_FLESHDEFAULT:
		SelectedParticles = FleshDefaultImpact;
		SelectedNiaParticles = FleshDefaultNiaEffect;
		SelectedSound = ImpactDefaultSound;
		break;
	case SURFACE_FLESHVITAL:
		SelectedParticles = FleshVitalImpact;
		SelectedSound = ImpactVitalSound;
		break;
	default:
		SelectedParticles = DefaultImpact;
		SelectedSound = ImpactDefaultSound;
		break;
	}
	if (SelectedParticles)
	{

		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleName);
		FVector Direction = TraceEndPoint - MuzzleLocation;
		Direction.Normalize();
		//UE_LOG(LogTemp, Warning, TEXT("impact"));
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			SelectedParticles,
			TraceEndPoint,
			Direction.Rotation(),
			FVector(0.5f, 0.5f, 0.5f)
		);
	}
	if (SelectedSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			SelectedSound,
			TraceEndPoint,
			FRotator::ZeroRotator
		);
	}

	if (FleshDefaultNiaEffect)
	{/*
		FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleName);
		FVector Direction = MuzzleLocation - TraceEndPoint;
		Direction.Normalize();*/
		FRotator dirRot = Direction.Rotation();
		UNiagaraComponent* FleshDefaultNiaComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			FleshDefaultNiaEffect,
			TraceEndPoint,
			dirRot);

		dirRot = dirRot.RotateVector(FVector(0, 0, 1)).Rotation();

		if (FleshDefaultNiaComp)
		{
			FleshDefaultNiaComp->SetWorldRotation(dirRot);
		}
	}

}

void ASWeapon::Reload()
{
	if (Ammo > 0)
	{
		int AmmoDelta = ClipSize - AmmoInClip;

		AmmoDelta = Ammo < AmmoDelta ? Ammo : AmmoDelta;
			
		Ammo -= AmmoDelta;

		AmmoInClip += AmmoDelta;
	}
}

void ASWeapon::StartFire()
{
	float delay = FMath::Max(lastTimeFired + TimeBetweenShots - GetWorld()->TimeSeconds,0.0f);

	GetWorldTimerManager().SetTimer(FireRate_TimerHandle,this, &ASWeapon::Fire, TimeBetweenShots,true, delay);
}

void ASWeapon::StopFire_Implementation()
{
	GetWorldTimerManager().ClearTimer(FireRate_TimerHandle);
	AccuracyJump = 0.0f;
}

void ASWeapon::PlayFireEffects(const FVector& TraceEndPoint)
{
	if (MuzzleEffect)
	{
		// debug line from camera to impact point


		UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleName);
	}
	
	if (TraceNiaEffect)
	{
		UNiagaraComponent* TraceNiaComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TraceNiaEffect,
			MeshComp,
			MuzzleName,
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true);
		if (TraceNiaComp)
		{
			TraceNiaComp->SetNiagaraVariableVec3(FString("BeamEnd"), TraceEndPoint);
		}
	}

	if (TraceEffect)
	{
		UParticleSystemComponent* TracerComp = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			TraceEffect,
			MeshComp->GetSocketLocation(MuzzleName)
		);

		if (TracerComp)
		{
			TracerComp->SetVectorParameter(TraceTargetName, TraceEndPoint);
		}
	}

	// Camera Shake
	APawn* MyOwner = Cast<APawn>(GetOwner());

	if (MyOwner)
	{
		APlayerController* PC = Cast<APlayerController>(MyOwner->GetController());

		if (PC)
		{
			PC->ClientPlayCameraShake(FireCamShake);
		}
	}
}

// Called every frame
void ASWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void ASWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(ASWeapon, HitScanTrace, COND_SkipOwner);
	DOREPLIFETIME(ASWeapon, FireRate_TimerHandle);
	DOREPLIFETIME(ASWeapon, Ammo);
	DOREPLIFETIME(ASWeapon, AmmoInClip);
}