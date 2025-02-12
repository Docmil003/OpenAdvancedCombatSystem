// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "ACSSettings.h"
//#include "PhysicsEngine/PhysicsSettings.h"
//#include "GameFramework/MovementComponent.h"
//#include "PhysicalMaterials/PhysicalMaterial.h"
//#include "PhysicsCoreTypes.h"
//#include "PhysicsEngine/BodySetup.h"
//#include "UObject/UObjectIterator.h"


UACSSettings* UACSSettings::DefaultSettings = nullptr;

#include UE_INLINE_GENERATED_CPP_BY_NAME(ACSSettings)

UACSSettings::UACSSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UACSSettings::PostInitProperties()
{
	Super::PostInitProperties();

#if WITH_EDITOR
	LoadDamageType();
	LoadAmmoType();
	LoadWeapons();
#endif

	// Uverride the core default settings with this one if its the CDO (the one edited in Project Settings)
	if (UACSSettings::Get() == this)
	{
		UACSSettings::SetDefaultSettings(this);
	}

}

void UACSSettings::SetDefaultSettings(UACSSettings* InSettings)
{
	DefaultSettings = InSettings;
}

#if WITH_EDITOR
bool UACSSettings::CanEditChange(const FProperty* Property) const
{
	bool bIsEditable = Super::CanEditChange(Property);

	return bIsEditable;
}

void UACSSettings::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}

void UACSSettings::LoadDamageType()
{
	// read "SurfaceType" defines and set meta data for the enum
	// find the enum
	UEnum* Enum = StaticEnum<EDamageType>();
	// we need this Enum
	check(Enum);

	const FString KeyName = TEXT("DisplayName");
	const FString HiddenMeta = TEXT("Hidden");
	const FString UnusedDisplayName = TEXT("Unused");

	// remainders, set to be unused
	for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex)
	{
		// if meta data isn't set yet, set name to "Unused" until we fix property window to handle this
		// make sure all hide and set unused first
		// if not hidden yet
		if (!Enum->HasMetaData(*HiddenMeta, EnumIndex))
		{
			Enum->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
			Enum->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
		}
	}

	for (auto Iter = DamageTypes.CreateConstIterator(); Iter; ++Iter)
	{
		// @todo only for editor
		Enum->SetMetaData(*KeyName, *Iter->Name.ToString(), Iter->Type);
		// also need to remove "Hidden"
		Enum->RemoveMetaData(*HiddenMeta, Iter->Type);
	}
}

void UACSSettings::LoadAmmoType()
{
	// read "SurfaceType" defines and set meta data for the enum
	// find the enum
	UEnum* Enum = StaticEnum<EAmmoType>();
	// we need this Enum
	check(Enum);

	const FString KeyName = TEXT("DisplayName");
	const FString HiddenMeta = TEXT("Hidden");
	const FString UnusedDisplayName = TEXT("Unused");

	// remainders, set to be unused
	for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex)
	{
		// if meta data isn't set yet, set name to "Unused" until we fix property window to handle this
		// make sure all hide and set unused first
		// if not hidden yet
		if (!Enum->HasMetaData(*HiddenMeta, EnumIndex))
		{
			Enum->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
			Enum->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
		}
	}

	for (auto Iter = AmmoTypes.CreateConstIterator(); Iter; ++Iter)
	{
		// @todo only for editor
		Enum->SetMetaData(*KeyName, *Iter->Name.ToString(), Iter->Type);
		// also need to remove "Hidden"
		Enum->RemoveMetaData(*HiddenMeta, Iter->Type);
	}
}

void UACSSettings::LoadWeapons()
{
	// read "SurfaceType" defines and set meta data for the enum
	// find the enum
	UEnum* Enum = StaticEnum<EWeapon>();
	// we need this Enum
	check(Enum);

	const FString KeyName = TEXT("DisplayName");
	const FString HiddenMeta = TEXT("Hidden");
	const FString UnusedDisplayName = TEXT("Unused");

	// remainders, set to be unused
	for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex)
	{
		// if meta data isn't set yet, set name to "Unused" until we fix property window to handle this
		// make sure all hide and set unused first
		// if not hidden yet
		if (!Enum->HasMetaData(*HiddenMeta, EnumIndex))
		{
			Enum->SetMetaData(*HiddenMeta, TEXT(""), EnumIndex);
			Enum->SetMetaData(*KeyName, *UnusedDisplayName, EnumIndex);
		}
	}

	for (auto Iter = Weapons.CreateConstIterator(); Iter; ++Iter)
	{
		// @todo only for editor
		Enum->SetMetaData(*KeyName, *Iter->Name.ToString(), Iter->Type);
		// also need to remove "Hidden"
		Enum->RemoveMetaData(*HiddenMeta, Iter->Type);
	}
}


#endif	// WITH_EDITOR
