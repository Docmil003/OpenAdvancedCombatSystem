// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ACSCoreWeapons.h"
#include "Runtime/UMG/Public/UMG.h"
#include "SlotsModificationsList.generated.h"

/**
 * This widget will contain all mods that can be added to this specific slot
 */
UCLASS()
class ACS_WEAPONS_API USlotsModificationsList : public UUserWidget
{
	GENERATED_BODY()
	

protected:
	//Element that will hold the modification slots
	UPROPERTY(BlueprintReadOnly, Category="Generation", meta = (BindWidget))
	UPanelWidget* Container;

	//Contains information about this slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modification", meta = (ExposeOnSpawn))
	FModificatorInfo SlotData;

	//Builds the widget after initializing the data
	UFUNCTION(BlueprintImplementableEvent, Category = "Generation")
	void Build();

public:

	//Initializes the slot data and builds the widget
	void Initiate(const FModificatorInfo& Data);
	void Initiate(const FText& SlotName, const int32 SlotIndex);

	FORCEINLINE UPanelWidget* GetContainer() const { return Container; }

	FORCEINLINE const FModificatorInfo& GetSlotData() const { return SlotData; }

};
