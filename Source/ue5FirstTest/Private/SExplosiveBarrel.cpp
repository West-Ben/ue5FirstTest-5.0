// Fill out your copyright notice in the Description page of Project Settings.


#include "SExplosiveBarrel.h"
#include "SHealthComponent.h"
#include "Kismet/GameplayStatics.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Net/UnrealNetwork.h"
#include "Sound/SoundCue.h"

// Sets default values
ASExplosiveBarrel::ASExplosiveBarrel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASExplosiveBarrel::OnHealthChanged);


	SMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SMesh->SetSimulatePhysics(true);

	SMesh->SetCollisionObjectType(ECC_PhysicsBody);
	RootComponent = SMesh;


	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForce->SetupAttachment(SMesh);
	RadialForce->Radius = 400.0f;
	RadialForce->bImpulseVelChange = true;
	RadialForce->bAutoActivate = false;
	RadialForce->bIgnoreOwningActor = true;

	ExplosionImpulse = 400.0f;
	DefaultDamage = 50.0f;
	DamageFalloff = 300.0f;

	bReplicates = true;
	SetReplicateMovement(true);
}


void ASExplosiveBarrel::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* Instegator, AActor* DamageCauser)
{
	if (bExploded)
	{
		return;
	}
	
	if (Health <= 0.0f)
	{
		ExplodeBarrel();

		RadialForce->FireImpulse();
		FVector BoostIntensity = FVector::UpVector * ExplosionImpulse;
		SMesh->AddImpulse(BoostIntensity, NAME_None, true);


		TArray<AActor*> ignores;
		ignores.Add(this);

		UGameplayStatics::ApplyRadialDamageWithFalloff(
			GetWorld(),
			DefaultDamage,
			DefaultDamage * 0.1f,
			GetActorLocation(),
			RadialForce->Radius,
			ExplosionImpulse,
			DamageFalloff,
			TypeOfDamage,
			ignores,
			this,
			Instegator
		);
	}

}

void ASExplosiveBarrel::ExplodeBarrel_Implementation()
{
	bExploded = true;

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());

	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			ExplosionSound,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}

	SMesh->SetMaterial(0, ExplodedMaterial);
}

void ASExplosiveBarrel::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASExplosiveBarrel, bExploded);
}
