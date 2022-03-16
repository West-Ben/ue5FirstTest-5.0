// Fill out your copyright notice in the Description page of Project Settings.


#include "SAIController.h"
#include "Perception/AISense.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "SHealthComponent.h"
#include "EngineUtils.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "Engine/TargetPoint.h"
#include "Engine/Engine.h"
#include <ue5FirstTest/ue5FirstTestCharacter.h>


ASAIController::ASAIController()
{
}

void ASAIController::BeginPlay()
{
	Super::BeginPlay();
}


void ASAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	BehaviorTick(DeltaTime);
	// TODO: behavior,

	// Can see player
	//* 1) if player seen move to location to shoot
	//* 2) if player out of range or lost sight clear target , move to last location
	//* 3) if need to reload, do it
	//* 4) if facing player and at location to shoot, SHOOT
	//* 5) if you are not facing the player you can see, face it
	
	// No player seen
	//8 1) if you are at a random location, face a random location
	//* 2) find a random location to move to
	//* 3) set timer to move to next random location


	return;
}

void ASAIController::BehaviorTick(float DeltaTime)
{

	if (self)
	{
		bool moving = GetMoveStatus() == EPathFollowingStatus::Moving;

		if (targetActor)
		{

			FVector directionToPlayer = targetActorDirection();
			USHealthComponent* health = Cast<USHealthComponent>(targetActor->GetComponentByClass(USHealthComponent::StaticClass()));

			if (targetActorDirection().Size() > MAX_TARGET_DISTANCE || health && health->bGetIsDead())
			{
				perception->ForgetActor(targetActor);
				targetActor = nullptr;
			}

			if (!GetWorld()->GetTimerManager().IsTimerActive(fireTimerhandle) )
			{
				/* move to shooting location */
				if (FindMoveToSpotEQS && !GetWorld()->GetTimerManager().IsTimerActive(MoveTimerhandle) && !moving)
				{
					GetWorld()->GetTimerManager().SetTimer(MoveTimerhandle, [&]()
						{
							FindMoveToLocation();
						}, 5, false);
				}

				/* Reload weapon */
				if (self->GetAmmonInClip() <= 0 && self->GetVelocity().Size() <= 1.0f)
				{
					self->BeginReload();
				}

				
				/* Shoot player */
				if (FVector::DotProduct((directionToPlayer / directionToPlayer.Size()), self->GetActorForwardVector()) >= 0.98f)
				{
					if (!self->bGetReload())
					{
						self->BeginZoom();
						SetFocus(targetActor);
						self->BeginFire();
						GetWorld()->GetTimerManager().SetTimer(fireTimerhandle, [&]()
							{
								self->EndFire();
								
								ClearFocus(EAIFocusPriority::Default);
								self->EndZoom();
							}, 
						1, false);
					}
				} 
				else
				{
					RotateSelf(DeltaTime, directionToPlayer.Rotation());
				}
			}
			

		}
		else
		{
			SetTargetActor();

			WonderLocation(moving);

			if (!moving)
			{
				RotateSelf(DeltaTime, FaceDirection.Rotation());
			}
		}


	}
}

void ASAIController::WonderLocation(bool moving)
{
	if (!targetActor)
	{
		if (FindWonderToSpotEQS && !GetWorld()->GetTimerManager().IsTimerActive(MoveTimerhandle) && !moving)
		{
			GetWorld()->GetTimerManager().SetTimer(MoveTimerhandle, [&]()
				{
					FindWonderToLocation();
					FaceDirection = self->GetActorForwardVector();
					FaceDirection += FVector(FMath::RandRange(0, 300), FMath::RandRange(0, 300), self->GetActorForwardVector().Z);
				}, 10, false);
		}

	}
}

void ASAIController::RotateSelf(float DeltaTime, FRotator direction)
{
	self->SetActorRotation(FMath::RInterpTo(self->GetActorRotation(), direction, DeltaTime, REACTION_TIME));
}


void ASAIController::SetSelf(APawn* pawnSelf)
{ 
	self = Cast<Aue5FirstTestCharacter>(pawnSelf);


	if (self)
	{
		if (self->GetComponentByClass(UPawnSensingComponent::StaticClass()))
		{
			Senses = Cast<UPawnSensingComponent>(self->GetComponentByClass(UPawnSensingComponent::StaticClass()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot find UPawnSensingComponent"), *self->GetHumanReadableName());
		}

		if (self->GetComponentByClass(UAIPerceptionComponent::StaticClass()))
		{
			perception = Cast<UAIPerceptionComponent>(self->GetComponentByClass(UAIPerceptionComponent::StaticClass()));
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("%s cannot find UAIPerceptionComponent"), *self->GetHumanReadableName());
		}

		self->GetHealthComponent()->TeamNum = uint8(50);
		WonderLocation(GetMoveStatus() == EPathFollowingStatus::Moving);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("cannot find Pawn"));
	}

}

void ASAIController::SetTargetActor()
{
	perception->GetKnownPerceivedActors(TSubclassOf<UAISense>(), PerceivedActors);

	if (!PerceivedActors.IsEmpty())
	{
		float closestActorDistance = 0.0f;
		float newDistance = 0.0f;
		
		for (int i = 0; i < PerceivedActors.Num(); i++)
		{

			if (!self->GetHealthComponent()->isFriendly(PerceivedActors[i],self))
			{
				USHealthComponent* health = Cast<USHealthComponent>(PerceivedActors[i]->GetComponentByClass(USHealthComponent::StaticClass()));

				if (health && health->GetHealth() > 0)
				{
					newDistance = FVector::Dist(PerceivedActors[i]->GetActorLocation(), self->GetActorLocation());
					if (closestActorDistance < newDistance)
					{
						targetActor = PerceivedActors[i];
						closestActorDistance = newDistance;

						PauseMove(FAIRequestID::CurrentRequest);

						UE_LOG(LogTemp, Warning,TEXT("%s"),*PerceivedActors[i]->GetFName().ToString());
						return;
					}
				}
			}
		}
	}
	if (targetActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("cleared %s"), *targetActor->GetFName().ToString());
		perception->ForgetActor(targetActor);
		targetActor = nullptr;
	}
	

	//GetPerceptionComponent()->
}

FVector ASAIController::targetActorDirection()
{
	if (targetActor)
	{
		return targetActor->GetActorLocation() - self->GetActorLocation();
	}
	return FVector(0, 0, 0);
}

float ASAIController::targetDestinationDistance()
{
	return (targetDestination - self->GetActorLocation()).Size();
}

void ASAIController::FindMoveToLocation()
{
	FEnvQueryRequest HidingSpotQueryRequest = FEnvQueryRequest(FindMoveToSpotEQS, self);
	
	HidingSpotQueryRequest.Execute(
		EEnvQueryRunMode::SingleResult,
		this,
		&ASAIController::bestQueryResult);
}

void ASAIController::bestQueryResult(TSharedPtr<FEnvQueryResult> result)
{
	if (result->IsSuccsessful())
	{
		targetDestination = result->GetItemAsLocation(0);
		AAIController::MoveToLocation(targetDestination);
	}
}

void ASAIController::FindWonderToLocation()
{
	FEnvQueryRequest HidingSpotQueryRequest = FEnvQueryRequest(FindWonderToSpotEQS, self);

	HidingSpotQueryRequest.Execute(
		EEnvQueryRunMode::SingleResult,
		this,
		&ASAIController::RandQueryResult);
}

void ASAIController::RandQueryResult(TSharedPtr<FEnvQueryResult> result)
{
	if (result->IsSuccsessful())
	{
		targetDestination = result->GetItemAsLocation(FMath::TruncToInt(FMath::RandRange(0,40)));
		AAIController::MoveToLocation(targetDestination);
	}
}

/*
void ASAIController::FindCoverLocation(FVector startLocation)
{
	if (!GetPawn()) { return; }

	FEnvQueryRequest coverLocation = FEnvQueryRequest(FindCoverPosition, GetPawn());

	coverLocation.Execute(
		EEnvQueryRunMode::SingleResult,
		this,
		&ASAIController::HandleCoverLocation
	);
	

}
*/

/*
void ASAIController::HandleCoverLocation(TSharedPtr result)
{
	if (result->IsSuccsessful())
	{
		MoveToLocation = result->GetItemAsLocation(0);
	}
}
*/

void ASAIController::SetFortLocation(AActor* actor)
{
	fortLocation = actor->GetActorLocation();
}