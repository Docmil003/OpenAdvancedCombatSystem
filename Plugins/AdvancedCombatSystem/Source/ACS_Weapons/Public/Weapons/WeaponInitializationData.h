// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "WeaponInitializationData.generated.h"

/**
 * Information use to save and load the weapon
 */
UCLASS(Blueprintable)
class ACS_WEAPONS_API UWeaponInitializationData : public UObject
{
	GENERATED_BODY()

public:

	/** Key byte is meant to be used as the slot where the modificator can be changed, while the value is the byte of the using component */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapons|Mods")
	TMap<uint8, uint8> Components = TMap<uint8, uint8>();

	//Ammount of ammo that has the weapon's magazine
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapons|Ammo")
	int32 AmmoInMagazine = 0;

	//Indicates if the weapon is jammed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Weapons|Jam")
	bool bJammed = false;

};
