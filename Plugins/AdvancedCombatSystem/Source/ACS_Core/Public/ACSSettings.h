// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ACS_CoreElements.h"
#include "ACSSettings.generated.h"

constexpr double movieRate = 1.0 / 24.0;

USTRUCT()
struct FDamageTypeName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "DamageTypeName")
	TEnumAsByte<enum EDamageType> Type;

	UPROPERTY(EditAnywhere, Category = "DamageTypeName")
	FName Name;

	FDamageTypeName()
		: Type(DamageType_Max)
	{}
	FDamageTypeName(EDamageType InType, const FName& InName)
		: Type(InType)
		, Name(InName)
	{}
};

USTRUCT()
struct FAmmoTypeName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "AmmoTypeName")
	TEnumAsByte<enum EAmmoType> Type;

	UPROPERTY(EditAnywhere, Category = "AmmoTypeName")
		FName Name;

	FAmmoTypeName()
		: Type(AmmoType_Max)
	{}
	FAmmoTypeName(EAmmoType InType, const FName& InName)
		: Type(InType)
		, Name(InName)
	{}
};

USTRUCT()
struct FWeaponName
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "WeaponTypeName")
	TEnumAsByte<enum EWeapon> Type;

	UPROPERTY(EditAnywhere, Category = "WeaponTypeName")
		FName Name;

	FWeaponName()
		: Type(Weapon_Max)
	{}
	FWeaponName(EWeapon InType, const FName& InName)
		: Type(InType)
		, Name(InName)
	{}
};

UCLASS(Config = Plugins, DefaultConfig, DisplayName = "Advanced Combat System")
class ACS_CORE_API UACSSettings : public UDeveloperSettings
{
	GENERATED_UCLASS_BODY()

public:

    // Uses the previous enumeration
	UPROPERTY(config, EditAnywhere, Category=Damages)
	TArray<FDamageTypeName> DamageTypes;

	// Uses the previous enumeration
	UPROPERTY(config, EditAnywhere, Category = Weapondry)
	TArray<FAmmoTypeName> AmmoTypes;

	// Uses the previous enumeration
	UPROPERTY(config, EditAnywhere, Category = Weapondry)
	TArray<FWeaponName> Weapons;

	//Indicates if bullets use a fixed-tick rate instead of using the actor's tick
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (DisplayName = "Projectiles Use Fixed Delta Seconds"))
    bool bBulletFixedTick = true;

	//Indicates the frequency refresh rate (tick) for the bullets for each frame
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Projectile, meta = (DisplayName = "Projectiles Fixed Delta Seconds Interval", EditCondition = "bBulletFixedTick"))
    double BulletFixedTick = movieRate;

	//Time whenever the pool system add a bullet for itself in case of need
    UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Pool)
    double PoolFillInterval = 1;

	//Just as the Bullet fix tick, this controls if beams should be updated as the frame rate or by a custom tick delta time
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Beam, meta = (DisplayName = "Beams Use Fixed Delta Seconds"))
	bool bBeamFixedTick = false;

	//Custom fixed-tick to update the beams with
	UPROPERTY(config, EditAnywhere, BlueprintReadOnly, Category = Beam, meta = (DisplayName = "Beams Fixed Delta Seconds Interval", EditCondition = "bBeamFixedTick"))
	double BeamFixedTick = movieRate;

public:
    static UACSSettings* Get() { return CastChecked<UACSSettings>(UACSSettings::StaticClass()->GetDefaultObject()); }
    

	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual bool CanEditChange(const FProperty* Property) const override;
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;

	/** Load Material Type data from INI file **/
	/** this changes displayname meta data. That means we won't need it outside of editor*/
	void LoadDamageType();
	void LoadAmmoType();
	void LoadWeapons();

#endif // WITH_EDITOR

protected:
	static void SetDefaultSettings(UACSSettings* InSettings);

private:

	// Override default settings.
	// This should be set up to point to the CDO of the leaf settings class (as edited in Project Settings)
	static UACSSettings* DefaultSettings;

};
