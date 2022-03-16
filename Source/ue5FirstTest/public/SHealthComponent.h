// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SHealthComponent.generated.h"

// broadcast on health change
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FONHealthChangedSignature, USHealthComponent*, HealthComp, float, Health, float, HealthDelta, const UDamageType*, damageType,AController*, Instegator, AActor*, DamageCauser);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5FIRSTTEST_API USHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USHealthComponent();

	UPROPERTY(EditDefaultsOnly, Replicated, BlueprintReadWrite, Category = "HealthComponent")
	uint8 TeamNum;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Health(float OldHealth);

	UPROPERTY(ReplicatedUsing = OnRep_Health, BlueprintReadOnly, Category = "HealthComponent")
	float Health;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HealthComponent")
	float MaxHealth;

	UFUNCTION()
	void HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* damageType, class AController* Instegator, AActor* DamageCauser);

	UPROPERTY()
	bool bIsDead;


public:

	float GetHealth() const { return Health; }


	bool bGetIsDead() const { return bIsDead; }

	UPROPERTY(BlueprintAssignable, Category="Events")
	FONHealthChangedSignature OnHealthChanged;

	UFUNCTION(BlueprintCallable,BlueprintPure, Category = "HealthComponent")
	static bool isFriendly(AActor* ActorA, AActor* ActorB);
};
