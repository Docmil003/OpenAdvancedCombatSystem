// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "ACS_WeaponsEditor.h"
#include "MessageLogModule.h"
#include "ACSSettingsDetails.h"

#define LOCTEXT_NAMESPACE "FACS_WeaponsEditorModule"

void FACS_WeaponsEditorModule::StartupModule()
{
	//LOAD PROPERTIES
	FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");

	PropertyModule.RegisterCustomClassLayout("ACSSettings", FOnGetDetailCustomizationInstance::CreateStatic(&FACSSettingsDetails::MakeInstance));

	//CREATE LOG CATEGORY
	FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
	FMessageLogInitializationOptions InitOptions;
	InitOptions.bShowPages = true;
	InitOptions.bAllowClear = true;
	InitOptions.bShowFilters = true;
	MessageLogModule.RegisterLogListing("AdvancedCombatSystem", NSLOCTEXT("AdvancedCombatSystem", "AdvancedCombatSystemLogLabel", "Advanced Combat System"), InitOptions);
}

void FACS_WeaponsEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
	{
		// unregister message log
		FMessageLogModule& MessageLogModule = FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
		MessageLogModule.UnregisterLogListing("AdvancedCombatSystem");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FACS_WeaponsEditorModule, ACS_WeaponsEditor)