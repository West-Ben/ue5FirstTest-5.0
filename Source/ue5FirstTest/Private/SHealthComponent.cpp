// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "../ue5FirstTest.h"
#include "Net/UnrealNetwork.h"
#include "SGameModeBase.h"

USHealthComponent::USHealthComponent()
{	
	MaxHealth = 100;
	bIsDead = false;
	TeamNum = 255;

	SetIsReplicatedByDefault(true);

	
}

void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	Health = MaxHealth;

	if (GetOwnerRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner)
		{
			MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
		}
	}


	
}

void USHealthComponent::OnRep_Health(float OldHealth)
{
	OnHealthChanged.Broadcast(this, Health, (Health - OldHealth), nullptr, nullptr, nullptr);
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const UDamageType* damageType, AController* Instegator, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) { return; }

	if (DamagedActor != DamageCauser && isFriendly(DamagedActor, DamageCauser)) { return; }

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);

	bIsDead = Health <= 0.0f;

	if (bIsDead)
	{
		ASGameModeBase* GM = Cast<ASGameModeBase>(GetWorld()->GetAuthGameMode());
		if (GM)
		{
			GM->OnActorKilled.Broadcast(GetOwner(), DamageCauser, Instegator);
		}
	}

	OnHealthChanged.Broadcast(this, Health, Damage, damageType, Instegator, DamageCauser);
		

}

bool USHealthComponent::isFriendly(AActor* ActorA, AActor* ActorB)
{
	if (ActorA == nullptr || ActorB == nullptr) { return true; }

	USHealthComponent* HealthCompA = Cast<USHealthComponent>(ActorA->GetComponentByClass(USHealthComponent::StaticClass()));
	USHealthComponent* HealthCompB = Cast<USHealthComponent>(ActorB->GetComponentByClass(USHealthComponent::StaticClass()));

	if (HealthCompA == nullptr || HealthCompB == nullptr) { return true; }

	return HealthCompA->TeamNum == HealthCompB->TeamNum;
}


void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
	DOREPLIFETIME(USHealthComponent, TeamNum);
}