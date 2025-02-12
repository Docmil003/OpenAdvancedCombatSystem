// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Widgets/Mods/SlotsModificationsList.h"

void USlotsModificationsList::Initiate(const FText& SlotName, const int32 SlotIndex)
{
	Initiate(FModificatorInfo(SlotName, SlotIndex));
}

void USlotsModificationsList::Initiate(const FModificatorInfo& Data)
{
	SlotData = Data;
	Build();
}
