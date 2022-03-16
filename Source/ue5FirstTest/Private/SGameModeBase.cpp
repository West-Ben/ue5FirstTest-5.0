// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameModeBase.h"
#include "TimerManager.h"
#include "SHealthComponent.h"
#include "SGameState.h"
#include "SPlayerState.h"
#include "EngineUtils.h"

ASGameModeBase::ASGameModeBase()
{
	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	NumberOfBots = 2;
	NumberOfWaves = 2;
	WaveCount = 0;
	BotCount = 0;
	timeBetweenWaves = 2.0f;

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;
	started = false;
}

void ASGameModeBase::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))	
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameModeBase::RestartDeadPlayers()
{
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();

		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);

		}
	}
}


void ASGameModeBase::StartPlay()
{
	Super::StartPlay();

	PrepareForNextWave();
}


void ASGameModeBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!started)
	{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
		{
			APlayerController* PC = It->Get();
			if (PC && PC->GetPawn())
			{
				started = true;
			}
		}
	}

	if (started)
	{
		CheckWaveState();
		CheckPlayerAlive();
	}
}


void ASGameModeBase::SpawnBot_TimerElapsed()
{
	//UE_LOG(LogTemp, Warning, TEXT("Spawning Bot"));
	SpawnBot();

	BotCount--;
	//UE_LOG(LogTemp, Warning, TEXT("Bot Count --"));

	if (BotCount <= 0)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Ending Wave"));
		EndWave();
	}
}

void ASGameModeBase::StartWave()
{
	WaveCount++;

	BotCount = NumberOfBots * WaveCount;

	GetWorldTimerManager().SetTimer(TimeHandle_BotSpawn, this, &ASGameModeBase::SpawnBot_TimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);
}

void ASGameModeBase::EndWave()
{
	GetWorldTimerManager().ClearTimer(TimeHandle_BotSpawn);


	SetWaveState(EWaveState::WaitingToComplete);
}

void ASGameModeBase::GameOver()
{
	EndWave();
	UE_LOG(LogTemp, Warning, TEXT("Game Over!"));
	SetWaveState(EWaveState::GameOver);
}

void ASGameModeBase::CheckWaveState()
{
	if (BotCount > 0 || GetWorldTimerManager().IsTimerActive(TimeHandle_NextWave))
	{ return; }

	bool bIsABotAlive = false;
	//
	for (TActorIterator<APawn> It(GetWorld()); It; ++It)
	{
		APawn* TestPawn = *It;
		if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
		{
			//UE_LOG(LogTemp, Warning, TEXT("IsPlayerControlled or null"));
			continue;
		}

		USHealthComponent* healthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));

		if (healthComp && healthComp->GetHealth() > 0.0f)
		{
			bIsABotAlive = true;
			break;
		}
	}

	if (!bIsABotAlive)
	{
		SetWaveState(EWaveState::WaveComplete);
		if (WaveCount < NumberOfWaves)
		{
			//UE_LOG(LogTemp, Warning, TEXT("prepare next wave in ChechWaveState"));
			PrepareForNextWave();
		}
	}
}

void ASGameModeBase::CheckPlayerAlive()
{
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; It++)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->GetPawn())
		{

			USHealthComponent* healthComp = Cast<USHealthComponent>(PC->GetPawn()->GetComponentByClass(USHealthComponent::StaticClass()));

			if (healthComp && healthComp->GetHealth() > 0.0f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Player alive"));
				return;
			}
			//UE_LOG(LogTemp, Warning, TEXT("Player dead"));
		}

	}
	//UE_LOG(LogTemp, Warning, TEXT("Player not alive or not found"));
	GameOver();
}

void ASGameModeBase::PrepareForNextWave()
{
	GetWorldTimerManager().SetTimer(TimeHandle_NextWave, this, &ASGameModeBase::StartWave, timeBetweenWaves, false);
	RestartDeadPlayers();
	SetWaveState(EWaveState::PreparingNextWave);
}


