// Copyright Epic Games, Inc. All Rights Reserved.

#include "ue5FirstTestGameMode.h"
#include "ue5FirstTestCharacter.h"
#include "UObject/ConstructorHelpers.h"

Aue5FirstTestGameMode::Aue5FirstTestGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
