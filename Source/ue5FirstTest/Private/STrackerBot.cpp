// Fill out your copyright notice in the Description page of Project Settings.


#include "STrackerBot.h"
#include "SHealthComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NavigationSystem.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "NavigationPath.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "../ue5FirstTestCharacter.h"
#include "Sound/SoundCue.h"

// Sets default values
ASTrackerBot::ASTrackerBot()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MovementForce = 1000.0f;
	bUseVelocityChange = true;
	RequiredDistance = 100.0f;
	MaxDistance = 3000.0f;
	Damage = 100.0f;
	ExplosionImpulse = 200.0f;
	JumpDistance = 300.0f;
	bStartedSelfDestruct = false;

	SMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	SMesh->SetCanEverAffectNavigation(false);
	SMesh->SetSimulatePhysics(true);
	RootComponent = SMesh;

	HealthComponent = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	HealthComponent->OnHealthChanged.AddDynamic(this, &ASTrackerBot::OnHealthChanged);

	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("RadialForceComp"));
	RadialForce->SetupAttachment(SMesh);
	RadialForce->Radius = 300.0f;
	RadialForce->bImpulseVelChange = true;
	RadialForce->bAutoActivate = false;
	RadialForce->bIgnoreOwningActor = true;


	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SphereComponent->SetSphereRadius(0.0f);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	SphereComponent->SetupAttachment(RootComponent);
	SphereComponent->bAutoActivate = false;

}

// Called when the game starts or when spawned
void ASTrackerBot::BeginPlay()
{
	Super::BeginPlay();
	
		//ACharacter* Playerpawn = UGameplayStatics::GetPlayerCharacter(this, 0);

		//Target = Cast<AActor>(Playerpawn);
		  
		//NextPath = GetNextPathPoint();
	//OnActorBeginOverlap.AddDynamic(this, &ASTrackerBot::NotifyActorBeginOverlap);
}

void ASTrackerBot::Destroyed()
{
	//if (bExploded) { return; }

	bExploded = true;

	TArray<AActor*> ignoredActors;
	ignoredActors.Add(this);

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionEffect, GetActorLocation());
	UGameplayStatics::ApplyRadialDamage(
		this,
		Damage,
		GetActorLocation(),
		300.0f,
		nullptr,
		ignoredActors,
		this,
		GetInstigatorController(),
		true);

	RadialForce->FireImpulse();
	if (ExplosionSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			GetWorld(),
			ExplosionSound,
			GetActorLocation(),
			FRotator::ZeroRotator
		);
	}
}

void ASTrackerBot::OnHealthChanged(USHealthComponent* OwningHealthComp, float Health, float HealthDelta, const UDamageType* DamageType, AController* Instegator, AActor* DamageCauser)
{
	if (MatInst == nullptr)
	{
		MatInst = SMesh->CreateAndSetMaterialInstanceDynamicFromMaterial(0, SMesh->GetMaterial(0));
	}

	MatInst->SetScalarParameterValue("LastTimeDamageTaken", GetWorld()->TimeSeconds);
	
	if (Health <= 0.0f)
	{
		Explode();
	}
}

FVector ASTrackerBot::GetNextPathPoint()
{
	if (Target)
	{
		UNavigationPath* NavPath = UNavigationSystemV1::FindPathToActorSynchronously(this, GetActorLocation(), Target);

		if (NavPath)
		{
			if (NavPath->PathPoints.Num() > 1)
			{
				return NavPath->PathPoints[1];
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ASTrackerBot NavPath is Null"));
		}

	}
	return GetActorLocation();
}

void ASTrackerBot::Explode()
{
	Destroy();
}


void ASTrackerBot::DamageSelf()
{
	UGameplayStatics::ApplyDamage(this, 20, GetInstigatorController(), this, nullptr);
}

// Called every frame
void ASTrackerBot::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


		if (Target)
		{
			float DistanceToNode = (GetActorLocation() - NextPath).Size();
			float DistanceToTarget = (GetActorLocation() - Target->GetActorLocation()).Size();

			if (GetLocalRole() == ROLE_Authority)
			{
				if (DistanceToTarget <= 200.0f)
				{
					if (!bStartedSelfDestruct)
					{
						Aue5FirstTestCharacter* targetPawn = Cast<Aue5FirstTestCharacter>(Target);


						if (targetPawn)
						{
							GetWorldTimerManager().SetTimer(TimerHandel_SelfDamage, this, &ASTrackerBot::DamageSelf, 0.25f, true, 0.0f);
							bStartedSelfDestruct = true;



						}
					}
				}
			}


			if (GetLocalRole() == ROLE_Authority)
			{
				if (DistanceToNode <= RequiredDistance)
				{
					NextPath = GetNextPathPoint();
				}
				else
				{

					if (DistanceToTarget <= JumpDistance && !bJumping)
					{
						JumpToTarget();
					}
					else if (DistanceToTarget > JumpDistance)
					{
						bJumping = false;
					}

					FVector ForceDirection = NextPath - GetActorLocation();
					ForceDirection.Normalize();

					ForceDirection *= MovementForce;

					SMesh->AddForce(ForceDirection, NAME_None, bUseVelocityChange);
					//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + ForceDirection, 32, FColor::Yellow, false, 0.0f, 0, 1.0f);

				}
				//DrawDebugSphere(GetWorld(), NextPath, 20, 12, FColor::Yellow, false, 4.0f, 1.0f);

				if (DistanceToTarget > MaxDistance)
				{
					//DrawDebugString(GetWorld(), GetActorLocation(), FString::Printf(TEXT("%s,%f"), *Target->GetName(), DistanceToTarget), (AActor*)0, FColor::White, 1.0f);
					Target = nullptr;
				}
			}
		}
		//if (DistanceToTarget <= RequiredDistance)
		//{
		//	Explode();
		//	FDamageEvent event;
		//	event.IsOfType(0);
		//	TakeDamage(100, event, GetInstigatorController(), this);
		//}
	
}

void ASTrackerBot::OnRep_SoundAlarm()
{
	if (SelfDestructWarningSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, SelfDestructWarningSound, GetActorLocation());
	}
}

void ASTrackerBot::JumpToTarget()
{
	TArray<AActor*> ignoredActors;
	ignoredActors.Add(this);
	FVector tossVelocity = FVector::ZeroVector;

	if (UGameplayStatics::SuggestProjectileVelocity(
		this,
		tossVelocity,
		GetActorLocation(),
		Target->GetActorLocation(),
		MovementForce,
		false,
		0.0f,
		0.0f,
		ESuggestProjVelocityTraceOption::OnlyTraceWhileAscending,
		FCollisionResponseParams::DefaultResponseParam,
		ignoredActors
	))
	{
		//DrawDebugString(GetWorld(), GetActorLocation(), FString::Printf(TEXT("%f,%f,%f"), tossVelocity.X, tossVelocity.Y, tossVelocity.Z), (AActor*)0, FColor::White, 1.0f);
		//DrawDebugDirectionalArrow(GetWorld(), GetActorLocation(), GetActorLocation() + tossVelocity * 100, 32, FColor::Yellow, false, 1.0f, 0, 1.0f);
		SMesh->SetPhysicsLinearVelocity(tossVelocity + Target->GetVelocity(),false, NAME_None);
		bJumping = true;
		JumpEffect();
	}

}

void ASTrackerBot::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Aue5FirstTestCharacter* checkActor = Cast<Aue5FirstTestCharacter>(OtherActor);
	if (checkActor)
	{
		USHealthComponent* HealthComp = Cast<USHealthComponent>(checkActor->GetComponentByClass(USHealthComponent::StaticClass()));

		if (HealthComp)
		{
			if (GetOwner() != OtherActor)
			{

				UE_LOG(LogTemp, Warning, TEXT("target: %s owner: %s"), *OtherActor->GetName(), *GetOwner()->GetName());

				Target = OtherActor;

				if (GetLocalRole() == ROLE_Authority)
				{
					NextPath = GetNextPathPoint();
				}
			}
		}
	}
}


void ASTrackerBot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASTrackerBot, bExploded);
	DOREPLIFETIME(ASTrackerBot, bJumping);
	DOREPLIFETIME(ASTrackerBot, bStartedSelfDestruct);
}