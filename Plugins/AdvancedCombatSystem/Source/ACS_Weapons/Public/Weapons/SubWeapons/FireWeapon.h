// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/Weapon.h"
#include "Templates/SubclassOf.h"
#include "FireWeapon.generated.h"

class UBulletPoolSystem;
class ABullet;

/**
 * A default implementated fire weapon actor
 */
UCLASS()
class ACS_WEAPONS_API  AFireWeapon : public AWeapon
{
	GENERATED_BODY()

public:

	AFireWeapon();

#pragma region Inherit
protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

#pragma region Bullet Data
protected:

	//Class of the bullet data structure to initialize the bullet with
	UPROPERTY(EditDefaultsOnly, Category = "Initialization", BlueprintGetter = GetInitializationDataClass, BlueprintSetter = SetInitializationDataClass)
	TSubclassOf<class UBulletInitiationSettings> BulletInitializationDataClass;

private:
	
	//Instance of the bullet initialization
	UPROPERTY(VisibleAnywhere, Category = "Initialization")
	TObjectPtr<class UBulletInitiationSettings> BulletInitiationSettings;

	//Class of bullet to spawn
	UPROPERTY(EditAnywhere, Category = "Initialization", BlueprintGetter = GetBulletClass, BlueprintSetter = SetBulletClass)
	TSubclassOf<ABullet> BulletClass;

//GETTERS & SETTERS
public:

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void SetInitializationDataClass(const TSubclassOf<UBulletInitiationSettings>& NewClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Initialization")
	FORCEINLINE const TSubclassOf<UBulletInitiationSettings>& GetInitializationDataClass() const { return BulletInitializationDataClass; };

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Initialization")
	TSubclassOf<class ABullet> GetBulletClass() const { return BulletClass; };

	UFUNCTION(BlueprintCallable, Category = "Initialization")
	void SetBulletClass(const TSubclassOf<ABullet>& NewBulletClass);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Initialization")
	FORCEINLINE const class UBulletInitiationSettings* const GetInitiationSettings() const { return BulletInitiationSettings.Get(); };

#pragma endregion

#pragma region Pool

private:

	//Handle method required for the bullet
	UPROPERTY(EditAnywhere, Category = "Pooling", BlueprintGetter = GetPoolingMethod, BlueprintSetter = SetPoolingMethod)
	TEnumAsByte<EPoolingMethod::Type> PoolingMethod = EPoolingMethod::Dynamic;

	//Minimun amount of bullets required in the pool (Only if Pooling method is Hybrid or fixed)
	UPROPERTY(EditAnywhere, Category = "Pooling", BlueprintGetter = GetMinBulletsInPool, BlueprintSetter = SetMinBulletsInPool, AdvancedDisplay, meta=(EditCondition="PoolingMethod == EPoolingMethod::Hybrid || PoolingMethod == EPoolingMethod::Fixed"))
	int32 MinBulletsInPool = 10;


//GETTERS & SETTERS
public:

	//Changes the requird bullets reserved, notifying the pool instance if necessary
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void SetMinBulletsInPool(const int32 NewMinBulletsInPool);

	//Gets the min of bullets reserved
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pooling")
	FORCEINLINE int32 GetMinBulletsInPool() const { return MinBulletsInPool; };

	//Changes the pool method notifying the pool instance if necessary
	UFUNCTION(BlueprintCallable, Category = "Pooling")
	void SetPoolingMethod(const TEnumAsByte<EPoolingMethod::Type> NewPoolingMethod);

	//Gets which pool method is been used
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pooling")
	FORCEINLINE TEnumAsByte<EPoolingMethod::Type> GetPoolingMethod() const { return PoolingMethod; };

#pragma endregion

#pragma region Shot

public:

	//Angle of deviation of the bullets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Aim")
	float RandomDeviation = 5;

	//Amount of bullets spawned per trigger. Usually going to be 1, but shotguns for example shot multiple pellets
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Shot")
	int32 BulletsPerShot = 1;

	//This function name is very self explanatory 
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Shot")
	void Shot();

	//Gets the settings for the bullet initialization
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Shot")
	void CreateBulletSettings(const int32 NBurst, UPARAM(ref) FBulletSettings& Settings);

	//Gets the transform from where bullets should spawn
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Shot")
	FTransform GetBulletSpawnTransform(); virtual FTransform GetBulletSpawnTransform_Implementation() { return GetActorTransform(); };


#pragma endregion

public:

	virtual AFireWeapon* GetFireWeapon_Implementation() override { return this; }

};
