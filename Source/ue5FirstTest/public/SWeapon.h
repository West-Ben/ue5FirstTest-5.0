// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SWeapon.generated.h"

class USkeletalMeshComponent;
class UDamageType;
class UParticleSystem;
class UCameraShakeBase;
class USoundBase;
class UNiagaraSystem;

// contains information for a single line trace
USTRUCT()
struct FHitScanTrace
{
	GENERATED_BODY()

public:

	UPROPERTY()
	TEnumAsByte<EPhysicalSurface> SurfaeType;

	UPROPERTY()
	FVector_NetQuantize TraceEnd;

	UPROPERTY()
	FVector_NetQuantize Direction;
};


UCLASS()
class UE5FIRSTTEST_API ASWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASWeapon();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual void StartFire();

	UFUNCTION(NetMulticast, Reliable,BlueprintCallable, Category = "Weapon")
	virtual void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual int GetAmmo() { return AmmoInClip; }

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	virtual bool bCanReload() { return AmmoInClip < ClipSize && Ammo > 0; }

	UFUNCTION()
	void PlayFireEffects(const FVector& TraceEndPoint);


	UFUNCTION()
	void PlayImpactEffects(const EPhysicalSurface& SurfaceType, const FVector& TraceEndPoint, const FVector& Direction);

	UFUNCTION()
	virtual void Reload();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USkeletalMeshComponent* MeshComp;

	virtual void Fire();

	UFUNCTION(Server, Reliable, WithValidation)
	virtual void ServerFire();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float FireRate;

	UPROPERTY(Replicated, BlueprintReadOnly)
	FTimerHandle FireRate_TimerHandle;

	float lastTimeFired;

	/* Derived from FireRare */
	float TimeBetweenShots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float NegativeAccuracy;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Replicated, Category = "Weapon")
	int Ammo;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	int ClipSize;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category = "Weapon")
	int AmmoInClip;

	/* Changes as fire is held. Resets on stopfire*/
	float AccuracyJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<UDamageType> DamageType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* MuzzleEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact Effect")
	UParticleSystem* DefaultImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact Effect")
	UParticleSystem* FleshVitalImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Impact Effect")
	UParticleSystem* FleshDefaultImpact;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UParticleSystem* TraceEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* TraceNiaEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	UNiagaraSystem* FleshDefaultNiaEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundBase* EmptySound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundBase* FireSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundBase* ImpactDefaultSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundBase* ImpactVitalSound;



	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<UCameraShakeBase> FireCamShake;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Range;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float VitalDamageMultiplier;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName MuzzleName;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName TraceTargetName;


	UPROPERTY(ReplicatedUsing=OnRep_HitScanTrace)
	FHitScanTrace HitScanTrace;

	UFUNCTION()
	void OnRep_HitScanTrace();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetAccuracy(float negAccuracy) { NegativeAccuracy = negAccuracy; }

	
};
