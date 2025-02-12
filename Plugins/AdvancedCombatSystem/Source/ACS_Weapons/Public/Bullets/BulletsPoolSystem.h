// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ACSCoreWeapons.h"
#include "BulletsPoolSystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUpdateClass, TSubclassOf<AActor>, Class);

class IACSBulletPoolingInterface;

/**
 * A class capable of handle all bullets in the level, preventing innecessary construct and destruction
 */
UCLASS(ClassGroup="AdvancedCombatSystem", AdvancedClassDisplay, meta=(ToolTip="Handles bullets' lifetime to prevent construction and deconstruction and increase performance even when bullets are been shot and destroy constantly", ShortToolTip="Pool for Bullets"))
class ACS_WEAPONS_API  UBulletsPoolSystem : public UObject
{
	GENERATED_BODY()

#pragma region CoreData

private:

	//Global and unique instance active of the pool
	static UBulletsPoolSystem* Instance;

	//A reference to the world to easily handle and access timers
	UPROPERTY()
	UWorld* World;
		
	//Array of the bullets handled
	UPROPERTY()
	TArray<AActor*> Pool;

	//Array of the bullets handled
	UPROPERTY()
	TArray<AActor*> Running;

	//Array of the bullets handled
	UPROPERTY()
	TArray<AActor*> Resting;
	
	//Indicates the amount of bullets required of a class
	UPROPERTY()
	TMap<TSubclassOf<AActor>, int32> PoolRequirements;
	
	//Indicates the amount of bullets existing of a class
	UPROPERTY()
	TMap<TSubclassOf<AActor>, int32> ClassConcurrency;


	//Interval between the adition of a bullet and another, used for fulfill the pool requirements
	static float FillingInterval;

	//A map that handles a timer for each Class of bullet that does not fulfill the requirements
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FTimerHandle> ClassesTimers;
	
public:

	UBulletsPoolSystem();

	virtual void BeginDestroy() override;

	virtual void ClearPools();

#pragma endregion

#pragma region BindFunctions
protected:

	/*
	* This is to bind it to when the bullet is destroyed and prevent non-accurated values in the amount of bullets
	*/
	UFUNCTION()
	void RemoveElement(AActor* Actor, EEndPlayReason::Type EndPlayReason);

#pragma endregion

#pragma region Blueprint Functions

public:

#pragma region Informational

	/*
	* Gets the current requirements of the pool system
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="Pool")
	TMap<TSubclassOf<AActor>, int32> GetRequirements() const { return PoolRequirements; };
	
	/*
	* Indicates if the ammount of bullets is the same or more than the required
	* @param Class class to check the concurrency
	* @return true if the amount is equal or more than required
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	bool IsPoolFull(TSubclassOf<AActor> const Class) const;
	

	/*
	* Indicates if the ammount of bullets is the same as the required
	* @param Class class to check the concurrency
	* @return true if the amount is the same as the required
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	bool IsClassOnLimit(TSubclassOf<AActor> const Class) const;
	

	/*
	* Gets the intance of the Bullets Pool System
	* @return the intance of the Bullets Pool System
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	static UBulletsPoolSystem* GetBulletsPoolInstance();

#pragma endregion

#pragma region Pooling

	/*
	* Creates a instance of the Bullets pool, should be created in the start of the game for safety (game instace, game mode, etc)
	* @param WorldContextObject It requires a world context object to be able to handle the pool correctly
	* @param Override If true, will destroy previous BulletPoolInstance, if false will only create a new instance if there's none
	* @return The created Instance of the Bullets Pool System
	*/
	UFUNCTION(BlueprintCallable, Category = "Pool", meta = (WorldContext = "WorldContextObject"))
	static const UBulletsPoolSystem* const CreateBulletsPoolInstance(UObject* WorldContextObject, bool Override);
	
	UFUNCTION(BlueprintCallable, Category = "Pool", meta = (WorldContext = "WorldContextObject"))
	void DestroyBulletsPoolInstance();

	/*
	* Requests and activates inactive bullets from the pool. Note that with some settings bullets can be also spawned
	* @param Transforms For each element will generate a bullet that copies its transform (world space)
	* @param Class Indicates what class of bullet should be spawned
	* @param Pooling Method of how generate the bullet
	* @param Bullets Array of the bullets that have been returned as the request
	* @param Settings Information to give to the bullet before activating it
	* @return true if all bullets requested have been spawned. If the ammount of bullets is less than the requested will be false (it can happen only with fixed pooling)
	*/
	UFUNCTION(BlueprintCallable, Category = "Pool")
	bool RequestElements(
		const TArray<FTransform>& Transforms,	const TSubclassOf<ABullet>& Class,
		const TEnumAsByte<EPoolingMethod::Type> Pooling,			TArray<AActor*>& Elements,
		const class UBulletInitiationSettings* Settings, AActor* NewOwner = nullptr
	);

	/*
	* Prevents the autodestruction of all the bullets of the class (Not affects subclasses)
	* @param Class the class of the bullets to prevent the despawning
	*/
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void PreventDespawningCurrentClass(TSubclassOf<AActor> const Class);


	/*
	* Updates the requirements for the pool. It calculates the diference between the new requirement and previous, so the requirements will be auto-calculated
	* @param Class Indicates the class that will have to change the concurrency
	* @param NewRequired The new amount of bullets (raw) that will be required
	* @param PreviousRequired The previous amount of bullets (raw) that are required before, the new value
	*/
	UFUNCTION(BlueprintCallable, Category = "Pool")
	void UpdatePoolRequisites(const TSubclassOf<AActor> Class, const int32 NewRequired, const int32 PreviusRequired = 0);
	

#pragma endregion

#pragma endregion

#pragma region Lifetime and Acces Functions

public:

	/*
	* Adds the bullet from the pool, meaning that will be managed
	* @param Bullet The bullet that will be added to the pool
	* @return True if the bullet was not in the pool, false otherwise
	*/
	
	bool AddElement(AActor* const Elem);

	/*
	* Removes the bullet from the pool, meaning that will no longer be managed
	* @param Bullet The bullet that will be removed from the pool
	* @return True if the bullet was in the pool, false otherwise
	*/
	bool RemoveElement(AActor* const Element);

	
	/*
	* Handles the activation of a bullet as well as its reflection in the maps variables
	* @param Bullet Bullet which will change its activation
	* @param bActive indicates if the bullet is going to be actived or not (deactived)
	* @return true if the bullet is been handle by the pooler
	*/
	bool SetActivation(AActor* const Element, const bool bActive);
	static bool ActivateElement(AActor* const Element, const FTransform& Transform, const class UBulletInitiationSettings* Settings, AActor* NewOwner, bool bIsHandled = true);
	static bool DeactivateElement(AActor* const Element);

	
	/*
	* Handles the map containing the info of the amount of bullets existing
	* @param BulletClass Class of the bullet to handle it's concurrency
	* @param bAdd indicates if the BulletClass have been removed or added
	*/
	void RegisterClass(TSubclassOf<AActor> const Class, const bool bAdd);
	
	/*
	* Starts a timer handle to control the amount of bullets
	* @param Class the class from which start the filling
	*/
	UFUNCTION()
	void InitiatePoolFilling(TSubclassOf<AActor> const Class);

	
	/*
	* Stops the timer that handles the filling
	* @param Class the class from which stop the filling
	*/
	void StopPoolFilling(TSubclassOf<AActor> const Class);

	
	/*
	* Adds or remove a bullet for fulfill the requirements of the pool requirements.
	* @param Class Specific class to try to fulfill the requirements
	*/
	void Fill(TSubclassOf<AActor> const Class);


	/**
	* Adds a group of bullets to be managed by the pool
	* @param Bullets Array of bullets to be managed
	* @return true if all bullets have been added, if one fails will return false
	*/
	UFUNCTION(BlueprintCallable, Category = "Pool")
	bool AddManyElements(TArray<AActor*> const Elements);

	/**
	* Gets an array of the classes that are been managed by the pool
	* @return An array of the classes that the pool manages
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	void GetClassesPooled(TArray<TSubclassOf<AActor>>& Classes) const;

	/**
	* Gets the bullets that are actived by class
	* @param Class Specific class to search through the active bullets
	* @param AllowSubclass Indicates if should include child classes of Class or otherwise, only accept the exact same class
	* @return An array of bullets actors actived from the specific class
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool", meta=(DeterminesOutputType="Class", AdvancedDisplay="AllowSubclass"))
	TArray<AActor*> GetActiveBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass = false) const;

	/**
	* Gets the bullets that are not actived by class
	* @param Class Specific class to search through the inactive bullets
	* @param AllowSubclass Indicates if should include child classes of Class or otherwise, only accept the exact same class
	* @return An array of bullets actors inactived from the specific class
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool", meta=(DeterminesOutputType="Class", AdvancedDisplay = "AllowSubclass"))
	TArray<AActor*> GetInactiveBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass = false) const;

	/**
	* Gets the bullets that are been pool handled by class
	* @param Class Specific class to search through the inactive bullets
	* @param AllowSubclass Indicates if should include child classes of Class or otherwise, only accept the exact same class
	* @return An array of bullets actors inactived from the specific class
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool", meta = (DeterminesOutputType = "Class", AdvancedDisplay = "AllowSubclass"))
	TArray<AActor*> GetPooledBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass = false) const;

	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FUpdateClass OnAddRequirement;

	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FUpdateClass OnRemoveRequirement;

	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FUpdateClass OnAddConcurrency;

	UPROPERTY(BlueprintAssignable, Category = "Pool")
	FUpdateClass OnRemoveConcurrency;

#pragma endregion

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	static bool IsInPool(AActor* Element);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pool")
	static bool IsActivated(AActor* Element);

};
