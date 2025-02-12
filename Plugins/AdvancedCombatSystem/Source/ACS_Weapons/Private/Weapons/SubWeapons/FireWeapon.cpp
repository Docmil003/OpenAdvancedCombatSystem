// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Weapons/SubWeapons/FireWeapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/ProjectileBehaviour.h"
#include "Bullets/Bullet.h"
#include "Bullets/BulletsPoolSystem.h"
#include "Bullets/BulletInitiationSettings.h"

#pragma region Utils Functions

//Indicates if with the specified method requires a min amount of bullets
inline bool RequiresMemoryReserve(const EPoolingMethod::Type Method) {
	return Method == EPoolingMethod::Fixed || Method == EPoolingMethod::Hybrid;
}

//Indicates if both methods handles coincide in the method requiring a min amount of bullets
inline bool SamePoolingManagement(const EPoolingMethod::Type A, const EPoolingMethod::Type B) {
	return (A == B) || RequiresMemoryReserve(A) == RequiresMemoryReserve(B);
}

#pragma endregion

AFireWeapon::AFireWeapon() : Super()
{
	BulletInitializationDataClass = UBulletInitiationSettings::StaticClass();
}

#pragma region Inherit

void AFireWeapon::BeginPlay()
{
	Super::BeginPlay();

	//Reserve bullets in pool
	if (RequiresMemoryReserve(PoolingMethod)) {
		if (UBulletsPoolSystem* Pool = UBulletsPoolSystem::GetBulletsPoolInstance()) {
			Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(BulletClass), MinBulletsInPool, 0);
		}
#if !NO_LOGGING
		else UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a bullet pool system instance"))
#endif
	}
	
	//Binds the trigger with the shot
	OnTriggeredDelegate.AddDynamic(this, &AFireWeapon::Shot);

	BulletInitiationSettings = NewObject<UBulletInitiationSettings>(this, BulletInitializationDataClass);

}

void AFireWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//Removes the bullets from the pool requirements
	SetPoolingMethod(EPoolingMethod::None);
	
}

#pragma endregion

#pragma region Bullet Data

void AFireWeapon::SetInitializationDataClass(const TSubclassOf<UBulletInitiationSettings>& NewClass)
{
	if (!NewClass) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Tried to update the initialization data class with a null class"))
		return; //Class must be valid
	}
	BulletInitializationDataClass = NewClass;
	BulletInitiationSettings = NewObject<UBulletInitiationSettings>(this, BulletInitializationDataClass);
}

void AFireWeapon::SetBulletClass(const TSubclassOf<ABullet>& NewBulletClass)
{
	if (!RequiresMemoryReserve(PoolingMethod)) return; 

	if (UBulletsPoolSystem* Pool = UBulletsPoolSystem::GetBulletsPoolInstance()) {

		//Sets the new requirement for previous bullet to 0 and adds it to the new class
		Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(BulletClass), 0, MinBulletsInPool);
		Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(NewBulletClass), MinBulletsInPool);
	}
#if !NO_LOGGING
	else UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a bullet pool system instance"))
#endif
	
	BulletClass = NewBulletClass;
}

#pragma endregion

#pragma region Pool

void AFireWeapon::SetMinBulletsInPool(int32 NewMinBulletsInPool)
{
	if (!RequiresMemoryReserve(PoolingMethod)) return;

	if (UBulletsPoolSystem* Pool = UBulletsPoolSystem::GetBulletsPoolInstance()) {

		Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(BulletClass), NewMinBulletsInPool, MinBulletsInPool);
	}
#if !NO_LOGGING
	else UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a bullet pool system instance"))
#endif
	
	MinBulletsInPool = NewMinBulletsInPool;
}

void AFireWeapon::SetPoolingMethod(TEnumAsByte<EPoolingMethod::Type> NewPoolingMethod)
{
	//If method do not differs on reserve method handle, there's no change
	if (SamePoolingManagement(PoolingMethod, NewPoolingMethod)) return;

	if (UBulletsPoolSystem* Pool = UBulletsPoolSystem::GetBulletsPoolInstance()) {

		if (RequiresMemoryReserve(NewPoolingMethod)) //when requires min pool reserve, creates it
			Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(BulletClass), MinBulletsInPool);
		
		else //when not, removes previous reserve
			Pool->UpdatePoolRequisites(StaticCast<TSubclassOf<AActor>>(BulletClass), 0, MinBulletsInPool);
		
	}
#if !NO_LOGGING
	else UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a bullet pool system instance"))
#endif

	PoolingMethod = NewPoolingMethod;
}

#pragma endregion

#pragma region Shot

void AFireWeapon::Shot_Implementation()
{
	//if pool not exists early return
	if (!UBulletsPoolSystem::GetBulletsPoolInstance())
	{
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a bullet pool system instance"))
		return;
	}

	TArray<FTransform> Trans;
	TArray<AActor*> Elems;

	//Initializes the array setting in all bullets that should spawn, the default transform
	Trans.Init(GetBulletSpawnTransform(), BulletsPerShot);

	//Generates the deviation for each bullet
	for (FTransform& i : Trans)
	{
		i.SetRotation(
			UKismetMathLibrary::RandomUnitVectorInConeInDegrees(
				i.GetRotation().Vector(), RandomDeviation
			).Rotation().Quaternion()
		);
	}

	CreateBulletSettings(GetCurrentBurstIndex(), BulletInitiationSettings->PhysicsSettings);

	//Requests the bullets with the information
	UBulletsPoolSystem::GetBulletsPoolInstance()->RequestElements(Trans, BulletClass, PoolingMethod, Elems, BulletInitiationSettings, this);

}

void AFireWeapon::CreateBulletSettings_Implementation(const int32 NBurst, FBulletSettings& Settings) {
	Settings = BulletClass->GetDefaultObject<ABullet>()->GetBulletComponent()->CurrentSettings;
};

#pragma endregion

