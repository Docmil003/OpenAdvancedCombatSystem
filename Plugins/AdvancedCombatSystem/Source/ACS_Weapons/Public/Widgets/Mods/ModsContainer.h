// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Runtime/UMG/Public/UMG.h"
#include "Runtime/UMG/Public/Blueprint/UserWidget.h"
#include "ACSCoreWeapons.h"
#include "SlotsModificationsList.h"
#include "ModButton.h"
#include "ModsContainer.generated.h"

/**
 * Base class to display the weapon slots an each one's modifications
 */
UCLASS()
class ACS_WEAPONS_API UModsContainer : public UUserWidget
{
	GENERATED_BODY()
	
	virtual void NativeConstruct() override;

#pragma region Generation
public:

	//Is the panel widget that will contain the modifications slot
	UPROPERTY(BlueprintReadOnly, Category = "Generation", meta=(BindWidget))
	UPanelWidget* Container;

	//Class that will be generated to represent each slot in the Container
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
	const TSubclassOf<USlotsModificationsList> SlotContainer = USlotsModificationsList::StaticClass();

	//Class that will be generated to represent each slot's mod in the property slot's Container
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generation")
	const TSubclassOf<UModButton> ModificationWidget = UModButton::StaticClass();

	/*
	* Populates the panel Container with the slots each one's modificators
	* @param Slots Is the enum of the modificable slots
	* @param ModList The array contains the mods of each slot. Each mod should be at the index that the intended slot represents
	*/
	UFUNCTION(BlueprintCallable, Category = "Generation")
	virtual void BuildWidget(const UEnum* Slots, const TArray<UEnum*>& ModList);

	/*
	* Generates the slot widget for the specified UEnum
	* @param Index represents which element of the enum entries should iterate with
	* @param Slots is the UEnum from which get the data
	* @param ModList Is the list of modificators tha should be created in the slots
	* @return The generated widget
	*/
	virtual USlotsModificationsList* GenerateSlot(int32 Index, const UEnum* Slots, const TArray<UEnum*>& ModList);

	/*
	* Generates the modificator widget for the specified UEnum
	* @param Index represents which element of the enum entries should iterate with
	* @param Modificator is the UEnum from which get the data
	* @param SlotInfo Is data from the owner slot to send when trigger the change
	* @return The generated widget
	*/
	virtual UModButton* GenerateMod(int32 Index, const UEnum* Modificator, const FModificatorInfo& SlotInfo);


#pragma endregion 

#pragma region Conditionals
public:

	//Indicates if the current enum can be succsesfully generated.
	virtual bool CanGenerateEnum(const UEnum* Enum, const int32 Index) const;

	//Indicates if the current Slot can be succsesfully generated.
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Generation")
	bool CanGenerateSlot(const UEnum* Enum, const int32 SlotIndex) const;

	//Indicates if the current Modificator can be succsesfully generated.
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Generation")
	bool CanGenerateMod(const UEnum* Enum, const int32 ModIndex) const;

#pragma endregion

#pragma region Delegate
public:

	//Triggers whenever gets the mod button gets activated
	UFUNCTION(BlueprintImplementableEvent, Category = "Generation")
	void OnModEditedEvent(const FModificatorInfo& SlotInfo, const FModificatorInfo& ModInfo);

	//Triggers whenever gets the mod button gets activated
	UPROPERTY(BlueprintAssignable)
	FEditModification OnModEdited;

#pragma endregion

};
