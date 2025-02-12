// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/MeleeComponent.h"
#include "Weapons/Weapon.h"
#include "ACS_CoreElements.h"
#include "MeleeWeapon.generated.h"

/**
 * The base class to create a Melee Based weapon
 */
UCLASS()
class ACS_WEAPONS_API  AMeleeWeapon : public AWeapon
{
	GENERATED_BODY()

#pragma region Main
public:

	//Activates the weapon
	UFUNCTION(BlueprintCallable, Category = "Melee")
	virtual void Activate(bool bReset);

	//Deactivates the weapon
	UFUNCTION(BlueprintCallable, Category = "Melee")
	virtual void Deactivate();

#pragma endregion

#pragma region GET ONE
public:

	/*Gets the melee component that contains the tag (if more than one will return the first found)*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	virtual UMeleeComponent* GetMeleeComponentByTag(const FName& Tag) const;

	//Gets the component that is active, even if there are many, will return the first that's active
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	UMeleeComponent* GetActiveMeleeComponent() const;

	//Gets the component that is active, even if there are many, will return the first that's active
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	UMeleeComponent* GetInactiveMeleeComponent() const;

#pragma endregion

#pragma region GET MULTIPLE
public:

	//Gets the component all the components that are currently active
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	void GetAllActiveMeleeComponents(TArray<UMeleeComponent*>& Components) const;

	//Gets the component all the components that are currently active
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	void GetAllInactiveMeleeComponents(TArray<UMeleeComponent*>& Components) const;

#pragma endregion

#pragma region INLINE GETTERS
public:

	//Gets all melee components
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	void GetAllMeleeComponents(TArray<UMeleeComponent*>& Components) const;

	//Indicates if the weapon can be activated
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee")
	bool CanActivate() const;
	virtual bool CanActivate_Implementation() const { return true; }

	//Indicates if the weapon can be deactivated
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Melee")
	bool CanDeactivate() const;
	virtual bool CanDeactivate_Implementation() const { return true; }

	/*Gets the melee component (if more than one will return the first found if not overriden)*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Melee")
	virtual UMeleeComponent* GetMeleeComponent() const { return GetComponentByClass<UMeleeComponent>(); };

#pragma endregion

public:

	virtual AMeleeWeapon* GetMeleeWeapon_Implementation() override { return this; }

};
