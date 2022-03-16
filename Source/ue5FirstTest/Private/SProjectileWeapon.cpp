// Fill out your copyright notice in the Description page of Project Settings.


#include "SProjectileWeapon.h"
#include "Kismet/GameplayStatics.h"


ASProjectileWeapon::ASProjectileWeapon()
{
}

void ASProjectileWeapon::Fire()
{

	AActor* WeaponOwner = GetOwner();
	if (WeaponOwner)
	{
		if (AmmoInClip > 0)
		{

			FVector EyeLocation;
			FRotator EyeRotation;

			WeaponOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

			FVector ShotDirection = EyeRotation.Vector();

			FVector MuzzleLocation = MeshComp->GetSocketLocation(MuzzleName);


			LaunchProjectile(MuzzleLocation, ShotDirection);

			//FActorSpawnParameters SpawnParams;

			//SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;


			//AActor* Projectile = GetWorld()->SpawnActor<AActor>(ProjectileClass, MuzzleLocation, EyeRotation, SpawnParams);
			//Projectile->SetOwner(GetOwner());

			if (FireSound)
			{
				UGameplayStatics::SpawnSoundAtLocation(
					GetWorld(),
					FireSound,
					GetActorLocation(),
					FRotator::ZeroRotator
				);
			}
			PlayFireEffects(FVector::ZeroVector);

			AmmoInClip--;
		}
		else
		{
			if (EmptySound)
			{
				UGameplayStatics::SpawnSoundAtLocation(
					GetWorld(),
					EmptySound,
					GetActorLocation(),
					FRotator::ZeroRotator
				);
			}
		}
	}
}