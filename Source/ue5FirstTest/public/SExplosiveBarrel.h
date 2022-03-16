// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SExplosiveBarrel.generated.h"

class USHealthComponent;
class UStaticMeshComponent;
class URadialForceComponent;
class UParticleSystem;
class UMaterialInterface;
class USoundCue;

UCLASS()
class UE5FIRSTTEST_API ASExplosiveBarrel : public AActor
{
	GENERATED_BODY()
	

public:	
	// Sets default values for this actor's properties
	ASExplosiveBarrel();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Components")
	class USHealthComponent* HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Components")
	class UStaticMeshComponent* SMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, category = "Components")
	class URadialForceComponent* RadialForce;

	UFUNCTION()
	void OnHealthChanged(USHealthComponent* OwningHealthComp, float Health,
		float HealthDelta, const class UDamageType* DamageType,
		class AController* Instegator, AActor* DamageCauser);

	UFUNCTION(NetMulticast, reliable)
	void ExplodeBarrel();


	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Damage")
	bool bExploded;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DefaultDamage;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float DamageFalloff;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	float ExplosionImpulse;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Sound Effect")
	USoundCue* ExplosionSound;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* ExplodedMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	TSubclassOf<UDamageType> TypeOfDamage;


	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimePrope) const override;
};
