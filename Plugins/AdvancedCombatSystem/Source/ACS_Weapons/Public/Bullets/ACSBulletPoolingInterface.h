// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ACSBulletPoolingInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UACSBulletPoolingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface used in the Advanced Combat System for add easy access to manage the activation and deactivation of the elements pooled
 */
class ACS_WEAPONS_API IACSBulletPoolingInterface
{
	GENERATED_BODY()

public:

	//Indicates that the bullet shall be deactivated and as inactive in the pool
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pool")
	void PutBulletToRest();

	/**
	* Activates the bullet to simulate a respawn with the received data
	* 
	* @param Transform Is the transform where the bullet will be "spawned"
	* @param Settings An instance of the settings that the bullet will read from to properly simulate
	* @return if the activation has been successfull
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pool")
	bool ActivateBullet(const FTransform& Transform, const class UBulletInitiationSettings* Settings);

	//Gets the max time that can be inactive before despawning
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pool")
	float GetInactiveLimit() const;

};
