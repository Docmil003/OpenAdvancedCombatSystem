// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponAccessInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponAccessInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * Interface for easy access
 */
class ACS_WEAPONS_API IWeaponAccessInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon", meta = (DisplayName = "GetWeapon"))
	FORCEINLINE class AWeapon* GetWeapon();
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon", meta = (DisplayName = "GetFireWeapon"))
	FORCEINLINE class AFireWeapon* GetFireWeapon();

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon", meta = (DisplayName = "GetMeleeWeapon"))
	FORCEINLINE class AMeleeWeapon* GetMeleeWeapon();


};
