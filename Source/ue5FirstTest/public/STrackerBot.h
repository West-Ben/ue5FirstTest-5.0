// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "STrackerBot.generated.h"

class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;
class UMaterialInterface;
class USHealthComponent;
class USphereComponent;
class Aue5FirstTestCharacter;
class USoundCue;

UCLASS()
class UE5FIRSTTEST_API ASTrackerBot : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASTrackerBot();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void Destroyed() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "TrackerBot")
	USHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "TrackerBot")
	USphereComponent* SphereComponent;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health,
		float HealthDelta, const class UDamageType* DamageType,
		class AController* Instegator, AActor* DamageCauser);

	UPROPERTY(VisibleAnywhere, Category = "TrackerBot")
	UStaticMeshComponent* SMesh;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	FVector NextPath;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MovementForce;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	bool bUseVelocityChange;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float RequiredDistance;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float JumpDistance;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float MaxDistance;

	UPROPERTY(EditDefaultsOnly, Category = "TrackerBot")
	float Damage;

	UFUNCTION(BlueprintCallable)
	FVector GetNextPathPoint();

	UMaterialInstanceDynamic* MatInst;

	UPROPERTY(Replicated,BlueprintReadOnly, Category = "Damage")
	bool bExploded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Components")
	class URadialForceComponent* RadialForce;

	UFUNCTION()
	void Explode();

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> TypeOfDamage;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Components")
	AActor* Target;

	UPROPERTY(Replicated, BlueprintReadOnly, category = "TrackerBot")
	bool bJumping;

	UPROPERTY(ReplicatedUsing=OnRep_SoundAlarm, BlueprintReadOnly, category = "TrackerBot")
	bool bStartedSelfDestruct;

	FTimerHandle TimerHandel_SelfDamage;

	UFUNCTION()
	void DamageSelf();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundCue* SelfDestructWarningSound;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimePrope) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnRep_SoundAlarm();

	UFUNCTION()
	void JumpToTarget();

	UFUNCTION(BlueprintImplementableEvent, Category = "FX")
	void JumpEffect();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor);
};
