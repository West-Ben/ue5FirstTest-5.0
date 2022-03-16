// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Templates/SharedPointer.h"
#include "Perception/PawnSensingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Templates/SharedPointer.h"
#include "Navigation/PathFollowingComponent.h"
#include "Tasks/AITask.h"
#include "../ue5FirstTestCharacter.h"
#include "EnvironmentQuery/Generators/EnvQueryGenerator_ProjectedPoints.h"
#include "SAIController.generated.h"

class UAISense;
class UEnvQuery;

/**
 *  Base AI controller for Character Enemies and Player Commanded AI
 */

UCLASS()
class UE5FIRSTTEST_API ASAIController : public AAIController
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
		class UPawnSensingComponent* Senses;

	UPROPERTY(VisibleAnywhere)
		class UAIPerceptionComponent* perception;

public:
	ASAIController();

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	void BehaviorTick(float DeltaTime);

	void WonderLocation(bool moving);



	//UFUNCTION(BlueprintCallable, Category = AI)
	//void FindCoverLocation(FVector startLocation);

	//UFUNCTION(BlueprintCallable, Category = AI)
	//void HandleCoverLocation(TSharedPtr<> result);

	UFUNCTION(BlueprintCallable, Category = AI)
		void FindMoveToLocation();

	UFUNCTION(BlueprintCallable, Category = AI)
		void FindWonderToLocation();

	UFUNCTION(BlueprintCallable, Category = AI)
		void SetFortLocation(AActor* actor);

	UFUNCTION(BlueprintCallable, Category = AI)
		void SetSelf(APawn* pawnSelf);

	UFUNCTION(BlueprintCallable, Category = AI)
		AActor* GetTargetActor() { return targetActor; }


	UFUNCTION(BlueprintCallable, Category = AI)
		FVector targetActorDirection();

	UFUNCTION(BlueprintCallable, Category = AI)
		float targetDestinationDistance();

	//UFUNCTION(BlueprintCallable, Category = AI)
	//FVector getMoveToLocation() { return MoveToLocation; }
	//TODO: Find Cover Function

	//TODO: Random Location and rotation, should look natural

	//TODO: Comunicate alerts to nerby team AI or Player if player commands

	//TODO: Handel firing and reloading on empty

	//TODO: taking cover on low health or out of ammo

	//TODO: Formations ones for Enemies and ones for Player commands

	//UPROPERTY(EditDefaultsOnly, Category = AI)
	//UEnvQuery* FindCoverPosition;

	UPROPERTY(EditAnywhere, Category = AI)
		UEnvQuery* FindMoveToSpotEQS;

	UPROPERTY(EditAnywhere, Category = AI)
		UEnvQuery* FindWonderToSpotEQS;



protected:
	void bestQueryResult(TSharedPtr<FEnvQueryResult> result);

	void RandQueryResult(TSharedPtr<FEnvQueryResult> result);

	UFUNCTION()
		void RotateSelf(float DeltaTime, FRotator direction);


	UFUNCTION()
		void SetTargetActor();

	FVector MoveToLocation;

	UPROPERTY()
		Aue5FirstTestCharacter* self;

	UPROPERTY()
		AActor* target;


	UPROPERTY()
		AActor* targetActor;
	UPROPERTY()
		FVector fortLocation;	
	UPROPERTY()
		FVector FaceDirection = FVector(0, 0, 0);
	UPROPERTY()
		FVector targetDestination = FVector(0,0,0);


	UPROPERTY()
		bool bReloading;

	UPROPERTY()
		bool bFiring;

	UPROPERTY(EditAnywhere, Category = AI)
		float MAX_TARGET_DISTANCE = 5000.0f;

	UPROPERTY(EditAnywhere, Category = AI)
		float REACTION_TIME = 10.0f;

	UPROPERTY(EditAnywhere, Category = AI)
		float ACCEPTABLE_MOVE_TO_DISTANCE = 100.0f;

	UPROPERTY()
		FTimerHandle fireTimerhandle;

	UPROPERTY()
		FTimerHandle MoveTimerhandle;

	UPROPERTY()
	TArray<AActor*> PerceivedActors;
};
