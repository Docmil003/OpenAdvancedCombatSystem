// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "BulletInitiationSettings.generated.h"

struct FBulletSettings;

/**
 * Holds all the necessary data to initiate a bullet with. Can be inhered to add more custom information that the bullet should have
 */
UCLASS(Blueprintable)
class ACS_WEAPONS_API UBulletInitiationSettings : public UObject
{
	GENERATED_BODY()
	
public:

	//Physics Settings for the bullet to initiate from
	UPROPERTY(EditAnywhere, Category = "Initialization")
	FBulletSettings PhysicsSettings;

};