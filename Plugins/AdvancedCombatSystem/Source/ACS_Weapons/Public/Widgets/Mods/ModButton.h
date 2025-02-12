// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ACSCoreWeapons.h"
#include "ModButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEditModification, const FModificatorInfo&, SlotInfo, const FModificatorInfo&, ModInfo);

/**
 * A default widget that represents a modification from a specific slot.
 */
UCLASS()
class ACS_WEAPONS_API UModButton : public UUserWidget
{
	GENERATED_BODY()
	
public:

	//Data to send about the slot where this mod is part of
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Generation", meta = (ExposeOnSpawn))
	FModificatorInfo SlotInfo;

	//Data to send about the modificator
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modification", meta = (ExposeOnSpawn))
	FModificatorInfo ModificationInfo;

	//Calls the delegate EditModification from the ModContainer widget with this button's information
	UFUNCTION(BlueprintCallable, Category = "Modification")
	void SendEdit() const;

	//Sets the data values and before calling the build function
	void Initiate(const FModificatorInfo& SlotInfo, const FModificatorInfo& ModInfo, FEditModification* Del);
	void Initiate(const FText& SlotName, const int32 SlotIndex, FText& ModificationName, int32 ModificationIndex, FEditModification* Del);

protected:

	//Builds the widget after receiving the information
	UFUNCTION(BlueprintImplementableEvent, Category = "Generation")
	void Build();

private:

	FEditModification* OnModEdited;


};
