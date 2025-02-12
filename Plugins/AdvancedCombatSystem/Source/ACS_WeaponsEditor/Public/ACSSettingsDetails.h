// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.



#pragma once



#include "Containers/Array.h"

#include "Containers/UnrealString.h"

#include "Delegates/Delegate.h"

#include "IDetailCustomization.h"

#include "Internationalization/Text.h"

#include "Templates/SharedPointer.h"

#include "Types/SlateEnums.h"

#include "Widgets/DeclarativeSyntaxSupport.h"

#include "Widgets/SCompoundWidget.h"



class IDetailLayoutBuilder;

class ITableRow;

class SEditableTextBox;

class STableViewBase;

class UEnum;

class UACSSettings;

struct FDamageTypeName;

struct FAmmoTypeName;

struct FWeaponName;

template <typename ItemType> class SListView;



DECLARE_DELEGATE(FOnCommitChangeDamage)



// Class containing the friend information - used to build the list view

class FDamageTypeListItem

{

public:



	/**
	* Constructor takes the required details
	*/

	FDamageTypeListItem(TSharedPtr<FDamageTypeName> InDamageType)

		: DamageType(InDamageType)

	{}



	TSharedPtr<FDamageTypeName> DamageType;

};



/**
* Implements the FriendsList
*/

class SDamageTypeEditBox : public SCompoundWidget

{

public:



	SLATE_BEGIN_ARGS(SDamageTypeEditBox) { }

		SLATE_ARGUMENT(TSharedPtr<FDamageTypeName>, DamageType)

		SLATE_ARGUMENT(UEnum*, DamageTypeEnum)

		SLATE_EVENT(FOnCommitChangeDamage, OnCommitChange)

	SLATE_END_ARGS()



public:



	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs);



	FString GetTypeString() const;



	FText GetName() const;

	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);

	void OnTextChanged(const FText& NewText);



private:

	TSharedPtr<FDamageTypeName>			DamageType;

	UEnum*								DamageTypeEnum;

	FOnCommitChangeDamage						OnCommitChange;

	TSharedPtr<SEditableTextBox>		NameEditBox;

};



typedef  SListView< TSharedPtr< FDamageTypeListItem > > SDamageTypeListView;







DECLARE_DELEGATE(FOnCommitChangeAmmo)



// Class containing the friend information - used to build the list view

class FAmmoTypeListItem

{

public:



	/**
	* Constructor takes the required details
	*/

	FAmmoTypeListItem(TSharedPtr<FAmmoTypeName> InAmmoType)

		: AmmoType(InAmmoType)

	{}



	TSharedPtr<FAmmoTypeName> AmmoType;

};



/**
* Implements the FriendsList
*/

class SAmmoTypeEditBox : public SCompoundWidget

{

public:



	SLATE_BEGIN_ARGS(SAmmoTypeEditBox) { }

	SLATE_ARGUMENT(TSharedPtr<FAmmoTypeName>, AmmoType)

		SLATE_ARGUMENT(UEnum*, AmmoTypeEnum)

		SLATE_EVENT(FOnCommitChangeAmmo, OnCommitChange)

		SLATE_END_ARGS()



public:



	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs);



	FString GetTypeString() const;



	FText GetName() const;

	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);

	void OnTextChanged(const FText& NewText);



private:

	TSharedPtr<FAmmoTypeName>			AmmoType;

	UEnum* AmmoTypeEnum;

	FOnCommitChangeAmmo						OnCommitChange;

	TSharedPtr<SEditableTextBox>		NameEditBox;

};



typedef  SListView< TSharedPtr< FAmmoTypeListItem > > SAmmoTypeListView;





































DECLARE_DELEGATE(FOnCommitChangeWeapon)



// Class containing the friend information - used to build the list view

class FWeaponsListItem

{

public:



	// Constructor takes the required details

	FWeaponsListItem(TSharedPtr<FWeaponName> InWeapons)

		: Weapons(InWeapons)

	{}



	TSharedPtr<FWeaponName> Weapons;

};



/**
* Implements the FriendsList
*/

class SWeaponsEditBox : public SCompoundWidget

{

public:



	SLATE_BEGIN_ARGS(SWeaponsEditBox) { }

	SLATE_ARGUMENT(TSharedPtr<FWeaponName>, Weapons)

		SLATE_ARGUMENT(UEnum*, WeaponsEnum)

		SLATE_EVENT(FOnCommitChangeWeapon, OnCommitChange)

		SLATE_END_ARGS()



public:

	/**
	* Constructs the application.
	* @param InArgs - The Slate argument list.
	**/

	void Construct(const FArguments& InArgs);



	FString GetTypeString() const;



	FText GetName() const;

	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);

	void OnTextChanged(const FText& NewText);



private:

	TSharedPtr<FWeaponName>			Weapons;

	UEnum* WeaponsEnum;

	FOnCommitChangeWeapon						OnCommitChange;

	TSharedPtr<SEditableTextBox>		NameEditBox;

};



typedef  SListView< TSharedPtr< FWeaponsListItem > > SWeaponsListView;







class FACSSettingsDetails : public IDetailCustomization

{

public:

	/** Makes a new instance of this detail layout class for a specific detail view requesting it */

	static TSharedRef<IDetailCustomization> MakeInstance();



	/** IDetailCustomization interface */

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;



private:



	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/

	TSharedRef<ITableRow> HandleGenerateListWidget(TSharedPtr< FDamageTypeListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> HandleGenerateListWidget(TSharedPtr< FAmmoTypeListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TSharedRef<ITableRow> HandleGenerateListWidget(TSharedPtr< FWeaponsListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);





private:

	TArray< TSharedPtr< FDamageTypeListItem > >	DamageTypeList;

	TArray< TSharedPtr< FAmmoTypeListItem > >	AmmoTypeList;

	TArray< TSharedPtr< FWeaponsListItem > >	DamagesList;



	UACSSettings* ACSSettings;

	UEnum* DamageTypeEnum;

	UEnum* AmmoTypeEnum;

	UEnum* WeaponsEnum;

	// functions

	void OnCommitChange();

};



