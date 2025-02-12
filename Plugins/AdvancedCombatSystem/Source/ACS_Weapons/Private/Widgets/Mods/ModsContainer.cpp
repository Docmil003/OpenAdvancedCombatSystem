// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Widgets/Mods/ModsContainer.h"
#include "Components/PanelWidget.h"

const FString HiddenMeta = TEXT("Hidden");

void UModsContainer::NativeConstruct()
{
	Super::NativeConstruct();

	OnModEdited.AddDynamic(this, &UModsContainer::OnModEditedEvent);

}

#pragma region Generation

void UModsContainer::BuildWidget(const UEnum* Slots, const TArray<UEnum*>& ModList)
{
	if (!Slots || ModList.IsEmpty()) return;

	//Populate main with the slots
	for (int32 SIndex = 0; SIndex < Slots->NumEnums(); ++SIndex) {
		if (CanGenerateSlot(Slots, SIndex))
			Container->AddChild(GenerateSlot(SIndex, Slots, ModList));
#if !NO_LOGGING
		else UE_LOG(AdvancedCombatSystem_Weapons_Log, Display, TEXT("Can't generate the slot %i from the enum %s"), SIndex, *Slots->GetName())
#endif
	}
}

USlotsModificationsList* UModsContainer::GenerateSlot(int32 Index, const UEnum* Slots, const TArray<UEnum*>& ModList)
{

	USlotsModificationsList* const SlotWidget = CreateWidget<USlotsModificationsList>(this, SlotContainer);

	FModificatorInfo SlotData(Slots->GetDisplayNameTextByIndex(Index), Index);

	SlotWidget->Initiate(SlotData);

	const UEnum* Modificator = ModList[Index];

	//Populate slot with the mods
	for (int32 ModIndex = 0; ModIndex < Modificator ->NumEnums(); ++ModIndex) {
		if (CanGenerateMod(Modificator, ModIndex)) 
			SlotWidget->GetContainer()->AddChild(GenerateMod(ModIndex, Modificator, SlotData));
		else UE_LOG(AdvancedCombatSystem_Weapons_Log, Display, TEXT("Can't generate the slot %i from the enum %s"), Modificator, *Modificator->GetName())
	}

	return SlotWidget;

}

UModButton* UModsContainer::GenerateMod(int32 Index, const UEnum* Modificator, const FModificatorInfo& SlotInfo)
{
	
	FModificatorInfo ModData(Modificator->GetDisplayNameTextByIndex(Index), Index);

	UModButton* const ModButton = CreateWidget<UModButton>(this, ModificationWidget);

	ModButton->Initiate(SlotInfo, ModData, &OnModEdited);

	return ModButton;

}

#pragma endregion

#pragma region Conditionals

bool UModsContainer::CanGenerateEnum(const UEnum* Enum, const int32 Index) const
{
	return Index < Enum->NumEnums() - 1
#if WITH_EDITORONLY_DATA //Metada exists if WITH_EDITORONLY_DATA
		&& !Enum->HasMetaData(*HiddenMeta, Index)
#endif
	; //semicolon here to affect independenly the preprocessor
}

bool UModsContainer::CanGenerateSlot_Implementation(const UEnum* Enum, const int32 SlotIndex) const
{
	return CanGenerateEnum(Enum, SlotIndex);
}

bool UModsContainer::CanGenerateMod_Implementation(const UEnum* Enum, const int32 ModIndex) const
{
	return CanGenerateEnum(Enum, ModIndex);
}

#pragma endregion