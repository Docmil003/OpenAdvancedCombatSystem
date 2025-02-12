// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.



#include "Bullets/Bullet.h"

#include "ACSCoreWeapons.h"

#include "Bullets/BulletsPoolSystem.h"

#include "Engine/World.h"

#include "TimerManager.h"

#include "Components/FakeBulletComponent.h"

#include "Components/ProjectileBehaviour.h"

#include "Bullets/BulletInitiationSettings.h"



DEFINE_LOG_CATEGORY(AdvancedCombatSystem_Weapons_Log)



typedef UBulletsPoolSystem UPool;



ABullet::ABullet()

{

	PrimaryActorTick.bCanEverTick = false;



	ProjectileComponent = CreateDefaultSubobject<UProjectileBehaviour>("ProjectileComponent");

	ProjectileComponent->SetAutoActivate(false);



}



#pragma region Initialization



void ABullet::BeginPlay()

{

	Super::BeginPlay();

	

	PutBulletToRest();



	//Bind delegates

#if !UE_BUILD_SHIPPING

	if (UFakeBulletComponent* Core = ProjectileComponent->GetCore())

	{

		Core->OnHit.AddDynamic(this, &ABullet::OnImpact);

		Core->OnBlockingHit.AddDynamic(this, &ABullet::OnBlock);

	}

	else {

		UE_LOG(AdvancedCombatSystem_Weapons_Log, Fatal, TEXT("This bullet does not have the Projectile Component or it's core"))

	}

#else

	ProjectileComponent->GetCore()->OnHit.AddDynamic(this, &ABullet::OnImpact);

	ProjectileComponent->GetCore()->OnBlockingHit.AddDynamic(this, &ABullet::OnBlock);

#endif



}



bool ABullet::RestInPool()

{

	return UPool::DeactivateElement(this);

}



#pragma endregion



#pragma region PoolControl



bool ABullet::ActivateBullet_Implementation(const FTransform& Transform, const UBulletInitiationSettings* Settings)

{



	IgnoreOwner();



	ProjectileComponent->CurrentSettings = Settings->PhysicsSettings;



	SetActorTransform(Transform);



	ProjectileComponent->Activate(true);



	if (DeactivationCountdown >= 0) {

		GetWorld()->GetTimerManager().SetTimer(AutoDeactiveTimer, this, &ABullet::PutBulletToRest, DeactivationCountdown, false);

	}



	//Shows the actor and prevents it from destroying

	SetActorHiddenInGame(false);

	SetLifeSpan(0);



	return true;

}



float ABullet::GetInactiveLimit_Implementation() const

{

	return InactiveLimit;;

}



void ABullet::PutBulletToRest_Implementation()

{

	RestInPool();

	SetActorHiddenInGame(true);

	ProjectileComponent->Deactivate();

	GetWorld()->GetTimerManager().ClearTimer(AutoDeactiveTimer);

}



#pragma endregion



#pragma region Owners manage



void ABullet::IgnoreOwner()

{

	if (!bIgnoreOwner) return;



	UFakeBulletComponent& Core = *ProjectileComponent->GetCore();



	AActor* CurrentOwner = GetOwner();



	do {

		Core.AddActorsToIgnore(CurrentOwner);

		CurrentOwner = CurrentOwner->GetOwner();



	} while (bIgnoreAllOwners && CurrentOwner); //If ignore all owners loop will execute once



}



#pragma endregion



#pragma region Events



void ABullet::OnImpact_Implementation(const FHitResult& Hit, const FVector& DesiredOffset, AActor* HitActor)

{

}



void ABullet::OnBlock_Implementation(const FHitResult& Hit, const FVector& DesiredOffset, AActor* HitActor)

{

	PutBulletToRest();

}



#pragma endregion

