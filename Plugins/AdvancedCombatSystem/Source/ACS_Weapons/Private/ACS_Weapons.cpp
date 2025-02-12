// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "ACS_Weapons.h"

#define LOCTEXT_NAMESPACE "FACS_WeaponsModule"

void FACS_WeaponsModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FACS_WeaponsModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FACS_WeaponsModule, ACS_Weapons)