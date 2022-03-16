// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "SGameModeBase.generated.h"

enum class EWaveState : uint8;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnActorKilled, AActor*, VictimActor, AActor*, KillerActor, AController*, Controller); // Killed Actor, killer actor, 

/**
 * 
 */
UCLASS()
class UE5FIRSTTEST_API ASGameModeBase : public AGameMode
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	
	UFUNCTION(BlueprintImplementableEvent,  Category = "GameMode")
	void SpawnBot();

	//UFUNCTION(BlueprintImplementableEvent, Category = "GameMode")
	void SpawnBot_TimerElapsed();

	void StartWave();

	void EndWave();

	void GameOver();

	void CheckWaveState();

	void CheckPlayerAlive();

	void PrepareForNextWave();

	FTimerHandle TimeHandle_BotSpawn;

	FTimerHandle TimeHandle_NextWave;

	virtual void StartPlay() override;
	
	UPROPERTY(BlueprintAssignable, Category = "GameMode")
	FOnActorKilled OnActorKilled;

protected:
	ASGameModeBase();


	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	int NumberOfBots;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	int NumberOfWaves;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameMode")
	float timeBetweenWaves;

	int WaveCount;

	int BotCount;

	void SetWaveState(EWaveState NewState);

	bool started;

	void RestartDeadPlayers();


};
