// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Widgets/Mods/ModButton.h"

void UModButton::SendEdit() const
{
	if (OnModEdited) OnModEdited->Broadcast(SlotInfo, ModificationInfo);
#if !NO_LOGGING
	else UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("For some reason the pointer to the delegate is unavaible"))
#endif
}

void UModButton::Initiate(const FModificatorInfo& InSlotInfo, const FModificatorInfo& InModInfo, FEditModification* Del)
{
	//Sets its information and prepares to build with data obtained
	SlotInfo = InSlotInfo;
	ModificationInfo = InModInfo;
	OnModEdited = Del;
	Build();
}

void UModButton::Initiate(const FText& SlotName, const int32 SlotIndex, FText& ModificationName, int32 ModificationIndex, FEditModification* Del)
{
	Initiate(
		FModificatorInfo(SlotName, SlotIndex),
		FModificatorInfo(ModificationName, ModificationIndex),
		Del
	);
}
