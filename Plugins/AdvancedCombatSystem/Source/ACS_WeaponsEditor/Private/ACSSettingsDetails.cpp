//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "ACSSettingsDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Fonts/SlateFontInfo.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailCustomNodeBuilder.h"
#include "IDocumentation.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/SToolTip.h"
#include "Widgets/Text/STextBlock.h"
#include "Containers/EnumAsByte.h"
#include "ACSSettings.h"
#include "Layout/Children.h"
#include "UObject/Class.h"
#include "UObject/NameTypes.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "UObject/UnrealNames.h"
#include "UObject/UnrealType.h"
#include "Chaos/ChaosEngineInterface.h"
#include "GenericPlatform/GenericPlatformMemory.h"
#include "HAL/PlatformCrt.h"
#include "HAL/PlatformMath.h"
#include "HAL/PlatformMisc.h"
#include "Internationalization/Internationalization.h"
#include "Misc/AssertionMacros.h"
#include "Misc/Attribute.h"
#include "Misc/MessageDialog.h"
#include "PropertyHandle.h"

#define LOCTEXT_NAMESPACE "DamageTypeDetails"

void SDamageTypeEditBox::Construct(const FArguments& InArgs)
{
	DamageType = InArgs._DamageType;
	DamageTypeEnum = InArgs._DamageTypeEnum;
	OnCommitChange = InArgs._OnCommitChange;
	check(DamageType.IsValid() && DamageTypeEnum);
	
	ChildSlot
	[
		SAssignNew(NameEditBox, SEditableTextBox)
		.Text(this, &SDamageTypeEditBox::GetName)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OnTextCommitted(this, &SDamageTypeEditBox::NewNameEntered)
		.OnTextChanged(this, &SDamageTypeEditBox::OnTextChanged)
		.IsReadOnly(DamageType->Type == DamageType_Default)
		.SelectAllTextWhenFocused(true)
	];
	
}

void SDamageTypeEditBox::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();
	
	if(NewName.Find(TEXT(" "))!=INDEX_NONE)
	{
		//No white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SDamageTypeEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	//Don't digest the number if we just clicked away from the pop-up
	if((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		FString NewName = NewText.ToString();
		if(NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			FName NewDamageName(*NewName);
			if(DamageType->Name != NAME_None && NewDamageName == NAME_None)
			{
				if(FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SDamageTypeListItem_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
				{
					return;
				}
			}
			
			if(NewDamageName != DamageType->Name)
			{
				DamageType->Name = NewDamageName;
				OnCommitChange.ExecuteIfBound();
			}
		}
		else
		{
			//clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SDamageTypeEditBox::GetName() const
{
	return FText::FromName(DamageType->Name);
}

class FDamageTypeList : public IDetailCustomNodeBuilder, public TSharedFromThis<FDamageTypeList>
{
public:
	
	FDamageTypeList(UACSSettings* InACSSetings, UEnum* InDamageTypeEnum, TSharedPtr<IPropertyHandle>& InDamageTypesProperty)
		: ACSSettings(InACSSetings)
		, DamageTypeEnum(InDamageTypeEnum)
		, DamageTypesProperty(InDamageTypesProperty)
	{
		DamageTypesProperty->MarkHiddenByCustomization();
	}
	
	void RefreshDamageTypeList()
	{
		//make sure no duplicate exists
		//if exists, use the last one
		
		for(auto Iter = ACSSettings->DamageTypes.CreateIterator(); Iter; ++Iter)
		{
			for(auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
			{
				//see if same type
				if(Iter->Type == InnerIter->Type)
				{
					//remove the current one
					ACSSettings->DamageTypes.RemoveAt(Iter.GetIndex());
					--Iter;
					break;
				}
			}
		}
		
		bool bCreatedItem[DamageType_Max];
		FGenericPlatformMemory::Memzero(bCreatedItem, sizeof(bCreatedItem));
		
		DamageTypeList.Empty();
		
		// I'm listing all of these because it is easier for users to understand how does this work.
		// I can't just link behind of scene because internally it will only save the enum
		// for example if you name DamageType5 to be Water and later changed to Sand, everything that used
		// DamageType5 will change to sand
		// I think what micht be better is to show what options they have, and it's for them to choose how to name
		
		//Add the first by default
		
		{
			bCreatedItem[0] = true;
			DamageTypeList.Add(MakeShareable(new FDamageTypeListItem(MakeShareable(new FDamageTypeName(DamageType_Default, TEXT("Default"))))));
		}
		
		//we don't create the first one. First one is always default.
		for(auto Iter = ACSSettings->DamageTypes.CreateIterator(); Iter; ++Iter)
		{
			bCreatedItem[Iter->Type] = true;
			DamageTypeList.Add(MakeShareable(new FDamageTypeListItem(MakeShareable(new FDamageTypeName(*Iter)))));
		}
		
		for(int32 Index = (int32)DamageType1; Index < DamageType_Max; ++Index)
		{
			if(bCreatedItem[Index] == false)
			{
				FDamageTypeName NeweElement((EDamageType)Index, TEXT(""));
				DamageTypeList.Add(MakeShareable(new FDamageTypeListItem(MakeShareable(new FDamageTypeName(NeweElement)))));
			}
		}
		
		//sort DamageListByType
		
		struct FCompareDamageType
		{
			FORCEINLINE bool operator()(const TSharedPtr<FDamageTypeListItem> A, const TSharedPtr<FDamageTypeListItem> B) const
			{
				check(A.IsValid());
				check(B.IsValid());
				return A->DamageType->Type < B->DamageType->Type;
			}
		};
		
		DamageTypeList.Sort(FCompareDamageType());
		
		ACSSettings->LoadDamageType();
		
		RegenerateChildren.ExecuteIfBound();
		
	}
	
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		RegenerateChildren = InOnRegenerateChildren;
	}
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		//no header row
	}
	
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		FText SearchString = LOCTEXT("FACSSettingsDetails_DamageType", "Damage Type");
		
		for (TSharedPtr<FDamageTypeListItem>& Item : DamageTypeList)
		{
			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(SearchString);
			
			FString TypeString = DamageTypeEnum->GetNameStringByValue((int64)Item->DamageType->Type);
			
			Row.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TypeString))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
			
			Row.ValueContent()
			[
				SNew(SDamageTypeEditBox)
				.DamageType(Item->DamageType)
				.DamageTypeEnum(DamageTypeEnum)
				.OnCommitChange(this, &FDamageTypeList::OnCommitChange)
			];
		}
	}
	
	virtual void Tick(float DeltaTime) override {}
	
	virtual bool RequiresTick() const override { return false; }
	
	virtual bool InitiallyCollapsed() const { return false; }
	
	virtual FName GetName() const override
	{
		static const FName Name(TEXT("DamageTypeList"));
		return Name;
	}
	
private:

	void OnCommitChange()
	{
		bool bDoCommit = true;
		
		//make sure it verifies all data is correct
		//skip the first one
		
		for(auto Iter = DamageTypeList.CreateConstIterator() + 1; Iter; ++Iter)
		{
			TSharedPtr<FDamageTypeListItem> ListItem = *Iter;
			if(ListItem->DamageType->Name != NAME_None)
			{
				//make sure no same name exists
				for(auto InnerIter = Iter+1; InnerIter; ++InnerIter)
				{
					TSharedPtr<FDamageTypeListItem> InnerItem = *InnerIter;
					if(ListItem->DamageType->Name == InnerItem->DamageType->Name)
					{
						//Duplicate name, warn user and get out
						FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FACSSettingsDetails_InvalidName", "Duplicate name found"));
						bDoCommit = false;
						break;
					}
				}
			}
		}
		
		if(bDoCommit)
		{
			DamageTypesProperty->NotifyPreChange();
			
			ACSSettings->DamageTypes.Empty();
			
			for(auto Iter = DamageTypeList.CreateConstIterator() + 1; Iter; ++Iter)
			{
				TSharedPtr<FDamageTypeListItem> ListItem = *Iter;
				if(ListItem->DamageType->Name != NAME_None)
				{
					ACSSettings->DamageTypes.Add(FDamageTypeName(ListItem->DamageType->Type, ListItem->DamageType->Name));
				}
			}
			
			ACSSettings->TryUpdateDefaultConfigFile();
			ACSSettings->LoadDamageType();
			
			DamageTypesProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
			
		}
		
	}
	
private:

	FSimpleDelegate RegenerateChildren;
	
	TArray<TSharedPtr<FDamageTypeListItem>> DamageTypeList;
	
	UACSSettings* ACSSettings;
	
	UEnum* DamageTypeEnum;
	
	TSharedPtr<IPropertyHandle> DamageTypesProperty;
	
	
};

#undef LOCTEXT_NAMESPACE

//AMMO

#define LOCTEXT_NAMESPACE "AmmoTypeDetails"


void SAmmoTypeEditBox::Construct(const FArguments& InArgs)
{
	AmmoType = InArgs._AmmoType;
	AmmoTypeEnum = InArgs._AmmoTypeEnum;
	OnCommitChange = InArgs._OnCommitChange;
	check(AmmoType.IsValid() && AmmoTypeEnum);
	
	ChildSlot
	[
		SAssignNew(NameEditBox, SEditableTextBox)
		.Text(this, &SAmmoTypeEditBox::GetName)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OnTextCommitted(this, &SAmmoTypeEditBox::NewNameEntered)
		.OnTextChanged(this, &SAmmoTypeEditBox::OnTextChanged)
		.IsReadOnly(AmmoType->Type == AmmoType_Default)
		.SelectAllTextWhenFocused(true)
	];
	
}

void SAmmoTypeEditBox::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();
	
	if(NewName.Find(TEXT(" "))!=INDEX_NONE)
	{
		//No white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SAmmoTypeEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	//Don't digest the number if we just clicked away from the pop-up
	if((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		FString NewName = NewText.ToString();
		if(NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			FName NewAmmoName(*NewName);
			if(AmmoType->Name != NAME_None && NewAmmoName == NAME_None)
			{
				if(FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SAmmoTypeListItem_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
				{
					return;
				}
			}
			
			if(NewAmmoName != AmmoType->Name)
			{
				AmmoType->Name = NewAmmoName;
				OnCommitChange.ExecuteIfBound();
			}
		}
		else
		{
			//clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SAmmoTypeEditBox::GetName() const
{
	return FText::FromName(AmmoType->Name);
}

class FAmmoTypeList : public IDetailCustomNodeBuilder, public TSharedFromThis<FAmmoTypeList>
{
public:
	
	FAmmoTypeList(UACSSettings* InACSSetings, UEnum* InAmmoTypeEnum, TSharedPtr<IPropertyHandle>& InAmmoTypesProperty)
		: ACSSettings(InACSSetings)
		, AmmoTypeEnum(InAmmoTypeEnum)
		, AmmoTypesProperty(InAmmoTypesProperty)
	{
		AmmoTypesProperty->MarkHiddenByCustomization();
	}
	
	void RefreshAmmoTypeList()
	{
		//make sure no duplicate exists
		//if exists, use the last one
		
		for(auto Iter = ACSSettings->AmmoTypes.CreateIterator(); Iter; ++Iter)
		{
			for(auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
			{
				//see if same type
				if(Iter->Type == InnerIter->Type)
				{
					//remove the current one
					ACSSettings->AmmoTypes.RemoveAt(Iter.GetIndex());
					--Iter;
					break;
				}
			}
		}
		
		bool bCreatedItem[AmmoType_Max];
		FGenericPlatformMemory::Memzero(bCreatedItem, sizeof(bCreatedItem));
		
		AmmoTypeList.Empty();
		
		// I'm listing all of these because it is easier for users to understand how does this work.
		// I can't just link behind of scene because internally it will only save the enum
		// for example if you name AmmoType5 to be Water and later changed to Sand, everything that used
		// AmmoType5 will change to sand
		// I think what micht be better is to show what options they have, and it's for them to choose how to name
		
		//Add the first by default
		
		{
			bCreatedItem[0] = true;
			AmmoTypeList.Add(MakeShareable(new FAmmoTypeListItem(MakeShareable(new FAmmoTypeName(AmmoType_Default, TEXT("Default"))))));
		}
		
		//we don't create the first one. First one is always default.
		for(auto Iter = ACSSettings->AmmoTypes.CreateIterator(); Iter; ++Iter)
		{
			bCreatedItem[Iter->Type] = true;
			AmmoTypeList.Add(MakeShareable(new FAmmoTypeListItem(MakeShareable(new FAmmoTypeName(*Iter)))));
		}
		
		for(int32 Index = (int32)AmmoType1; Index < AmmoType_Max; ++Index)
		{
			if(bCreatedItem[Index] == false)
			{
				FAmmoTypeName NeweElement((EAmmoType)Index, TEXT(""));
				AmmoTypeList.Add(MakeShareable(new FAmmoTypeListItem(MakeShareable(new FAmmoTypeName(NeweElement)))));
			}
		}
		
		//sort AmmoListByType
		
		struct FCompareAmmoType
		{
			FORCEINLINE bool operator()(const TSharedPtr<FAmmoTypeListItem> A, const TSharedPtr<FAmmoTypeListItem> B) const
			{
				check(A.IsValid());
				check(B.IsValid());
				return A->AmmoType->Type < B->AmmoType->Type;
			}
		};
		
		AmmoTypeList.Sort(FCompareAmmoType());
		
		ACSSettings->LoadAmmoType();
		
		RegenerateChildren.ExecuteIfBound();
		
	}
	
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		RegenerateChildren = InOnRegenerateChildren;
	}
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		//no header row
	}
	
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		FText SearchString = LOCTEXT("FACSSettingsDetails_AmmoType", "Ammo Type");
		
		for (TSharedPtr<FAmmoTypeListItem>& Item : AmmoTypeList)
		{
			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(SearchString);
			
			FString TypeString = AmmoTypeEnum->GetNameStringByValue((int64)Item->AmmoType->Type);
			
			Row.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TypeString))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
			
			Row.ValueContent()
			[
				SNew(SAmmoTypeEditBox)
				.AmmoType(Item->AmmoType)
				.AmmoTypeEnum(AmmoTypeEnum)
				.OnCommitChange(this, &FAmmoTypeList::OnCommitChange)
			];
		}
	}
	
	virtual void Tick(float DeltaTime) override {}
	
	virtual bool RequiresTick() const override { return false; }
	
	virtual bool InitiallyCollapsed() const { return false; }
	
	virtual FName GetName() const override
	{
		static const FName Name(TEXT("AmmoTypeList"));
		return Name;
	}
	
private:

	void OnCommitChange()
	{
		bool bDoCommit = true;
		
		//make sure it verifies all data is correct
		//skip the first one
		
		for(auto Iter = AmmoTypeList.CreateConstIterator() + 1; Iter; ++Iter)
		{
			TSharedPtr<FAmmoTypeListItem> ListItem = *Iter;
			if(ListItem->AmmoType->Name != NAME_None)
			{
				//make sure no same name exists
				for(auto InnerIter = Iter+1; InnerIter; ++InnerIter)
				{
					TSharedPtr<FAmmoTypeListItem> InnerItem = *InnerIter;
					if(ListItem->AmmoType->Name == InnerItem->AmmoType->Name)
					{
						//Duplicate name, warn user and get out
						FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FACSSettingsDetails_InvalidName", "Duplicate name found"));
						bDoCommit = false;
						break;
					}
				}
			}
		}
		
		if(bDoCommit)
		{
			AmmoTypesProperty->NotifyPreChange();
			
			ACSSettings->AmmoTypes.Empty();
			
			for(auto Iter = AmmoTypeList.CreateConstIterator() + 1; Iter; ++Iter)
			{
				TSharedPtr<FAmmoTypeListItem> ListItem = *Iter;
				if(ListItem->AmmoType->Name != NAME_None)
				{
					ACSSettings->AmmoTypes.Add(FAmmoTypeName(ListItem->AmmoType->Type, ListItem->AmmoType->Name));
				}
			}
			
			ACSSettings->TryUpdateDefaultConfigFile();
			ACSSettings->LoadAmmoType();
			
			AmmoTypesProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
			
		}
		
	}
	
private:

	FSimpleDelegate RegenerateChildren;
	
	TArray<TSharedPtr<FAmmoTypeListItem>> AmmoTypeList;
	
	UACSSettings* ACSSettings;
	
	UEnum* AmmoTypeEnum;
	
	TSharedPtr<IPropertyHandle> AmmoTypesProperty;
	
	
};

#undef LOCTEXT_NAMESPACE

//WEAPONS

#define LOCTEXT_NAMESPACE "WeaponsDetails"


void SWeaponsEditBox::Construct(const FArguments& InArgs)
{
	Weapons = InArgs._Weapons;
	WeaponsEnum = InArgs._WeaponsEnum;
	OnCommitChange = InArgs._OnCommitChange;
	check(Weapons.IsValid() && WeaponsEnum);
	
	ChildSlot
	[
		SAssignNew(NameEditBox, SEditableTextBox)
		.Text(this, &SWeaponsEditBox::GetName)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.OnTextCommitted(this, &SWeaponsEditBox::NewNameEntered)
		.OnTextChanged(this, &SWeaponsEditBox::OnTextChanged)
		.IsReadOnly(Weapons->Type == Weapon_Default)
		.SelectAllTextWhenFocused(true)
	];
	
}

void SWeaponsEditBox::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();
	
	if(NewName.Find(TEXT(" "))!=INDEX_NONE)
	{
		//No white space
		NameEditBox->SetError(TEXT("No white space is allowed"));
	}
	else
	{
		NameEditBox->SetError(TEXT(""));
	}
}

void SWeaponsEditBox::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	//Don't digest the number if we just clicked away from the pop-up
	if((CommitInfo == ETextCommit::OnEnter) || (CommitInfo == ETextCommit::OnUserMovedFocus))
	{
		FString NewName = NewText.ToString();
		if(NewName.Find(TEXT(" ")) == INDEX_NONE)
		{
			FName NewWeaponsName(*NewName);
			if(Weapons->Name != NAME_None && NewWeaponsName == NAME_None)
			{
				if(FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("SWeaponsListItem_DeleteConfirm", "Would you like to delete the name? If this type is used, it will invalidate the usage.")) == EAppReturnType::No)
				{
					return;
				}
			}
			
			if(NewWeaponsName != Weapons->Name)
			{
				Weapons->Name = NewWeaponsName;
				OnCommitChange.ExecuteIfBound();
			}
		}
		else
		{
			//clear error
			NameEditBox->SetError(TEXT(""));
		}
	}
}

FText SWeaponsEditBox::GetName() const
{
	return FText::FromName(Weapons->Name);
}

class FWeaponsList : public IDetailCustomNodeBuilder, public TSharedFromThis<FWeaponsList>
{
public:
	
	FWeaponsList(UACSSettings* InACSSetings, UEnum* InWeaponsEnum, TSharedPtr<IPropertyHandle>& InWeaponsProperty)
		: ACSSettings(InACSSetings)
		, WeaponsEnum(InWeaponsEnum)
		, WeaponsProperty(InWeaponsProperty)
	{
		WeaponsProperty->MarkHiddenByCustomization();
	}
	
	void RefreshWeaponsList()
	{
		//make sure no duplicate exists
		//if exists, use the last one
		
		for(auto Iter = ACSSettings->Weapons.CreateIterator(); Iter; ++Iter)
		{
			for(auto InnerIter = Iter + 1; InnerIter; ++InnerIter)
			{
				//see if same type
				if(Iter->Type == InnerIter->Type)
				{
					//remove the current one
					ACSSettings->Weapons.RemoveAt(Iter.GetIndex());
					--Iter;
					break;
				}
			}
		}
		
		bool bCreatedItem[Weapon_Max];
		FGenericPlatformMemory::Memzero(bCreatedItem, sizeof(bCreatedItem));
		
		WeaponsList.Empty();
		
		// I'm listing all of these because it is easier for users to understand how does this work.
		// I can't just link behind of scene because internally it will only save the enum
		// for example if you name Weapons5 to be Water and later changed to Sand, everything that used
		// Weapons5 will change to sand
		// I think what micht be better is to show what options they have, and it's for them to choose how to name
		
		//Add the first by default
		
		{
			bCreatedItem[0] = true;
			WeaponsList.Add(MakeShareable(new FWeaponsListItem(MakeShareable(new FWeaponName(Weapon_Default, TEXT("Default"))))));
		}
		
		//we don't create the first one. First one is always default.
		for(auto Iter = ACSSettings->Weapons.CreateIterator(); Iter; ++Iter)
		{
			bCreatedItem[Iter->Type] = true;
			WeaponsList.Add(MakeShareable(new FWeaponsListItem(MakeShareable(new FWeaponName(*Iter)))));
		}
		
		for(int32 Index = (int32)Weapon1; Index < Weapon_Max; ++Index)
		{
			if(bCreatedItem[Index] == false)
			{
				FWeaponName NeweElement((EWeapon)Index, TEXT(""));
				WeaponsList.Add(MakeShareable(new FWeaponsListItem(MakeShareable(new FWeaponName(NeweElement)))));
			}
		}
		
		//sort WeaponsListByType
		
		struct FCompareWeapons
		{
			FORCEINLINE bool operator()(const TSharedPtr<FWeaponsListItem> A, const TSharedPtr<FWeaponsListItem> B) const
			{
				check(A.IsValid());
				check(B.IsValid());
				return A->Weapons->Type < B->Weapons->Type;
			}
		};
		
		WeaponsList.Sort(FCompareWeapons());
		
		ACSSettings->LoadWeapons();
		
		RegenerateChildren.ExecuteIfBound();
		
	}
	
	virtual void SetOnRebuildChildren(FSimpleDelegate InOnRegenerateChildren) override
	{
		RegenerateChildren = InOnRegenerateChildren;
	}
	
	virtual void GenerateHeaderRowContent(FDetailWidgetRow& NodeRow) override
	{
		//no header row
	}
	
	virtual void GenerateChildContent(IDetailChildrenBuilder& ChildrenBuilder) override
	{
		FText SearchString = LOCTEXT("FACSSettingsDetails_Weapons", "Weapons Type");
		
		for (TSharedPtr<FWeaponsListItem>& Item : WeaponsList)
		{
			FDetailWidgetRow& Row = ChildrenBuilder.AddCustomRow(SearchString);
			
			FString TypeString = WeaponsEnum->GetNameStringByValue((int64)Item->Weapons->Type);
			
			Row.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TypeString))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
			
			Row.ValueContent()
			[
				SNew(SWeaponsEditBox)
				.Weapons(Item->Weapons)
				.WeaponsEnum(WeaponsEnum)
				.OnCommitChange(this, &FWeaponsList::OnCommitChange)
			];
		}
	}
	
	virtual void Tick(float DeltaTime) override {}
	
	virtual bool RequiresTick() const override { return false; }
	
	virtual bool InitiallyCollapsed() const { return false; }
	
	virtual FName GetName() const override
	{
		static const FName Name(TEXT("WeaponsList"));
		return Name;
	}
	
private:

	void OnCommitChange()
	{
		bool bDoCommit = true;
		
		//make sure it verifies all data is correct
		//skip the first one
		
		for(auto Iter = WeaponsList.CreateConstIterator() + 1; Iter; ++Iter)
		{
			TSharedPtr<FWeaponsListItem> ListItem = *Iter;
			if(ListItem->Weapons->Name != NAME_None)
			{
				//make sure no same name exists
				for(auto InnerIter = Iter+1; InnerIter; ++InnerIter)
				{
					TSharedPtr<FWeaponsListItem> InnerItem = *InnerIter;
					if(ListItem->Weapons->Name == InnerItem->Weapons->Name)
					{
						//Duplicate name, warn user and get out
						FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("FACSSettingsDetails_InvalidName", "Duplicate name found"));
						bDoCommit = false;
						break;
					}
				}
			}
		}
		
		if(bDoCommit)
		{
			WeaponsProperty->NotifyPreChange();
			
			ACSSettings->Weapons.Empty();
			
			for(auto Iter = WeaponsList.CreateConstIterator() + 1; Iter; ++Iter)
			{
				TSharedPtr<FWeaponsListItem> ListItem = *Iter;
				if(ListItem->Weapons->Name != NAME_None)
				{
					ACSSettings->Weapons.Add(FWeaponName(ListItem->Weapons->Type, ListItem->Weapons->Name));
				}
			}
			
			ACSSettings->TryUpdateDefaultConfigFile();
			ACSSettings->LoadWeapons();
			
			WeaponsProperty->NotifyPostChange(EPropertyChangeType::ValueSet);
			
		}
		
	}
	
private:

	FSimpleDelegate RegenerateChildren;
	
	TArray<TSharedPtr<FWeaponsListItem>> WeaponsList;
	
	UACSSettings* ACSSettings;
	
	UEnum* WeaponsEnum;
	
	TSharedPtr<IPropertyHandle> WeaponsProperty;
	
	
};

//FACSSettingsDetails

TSharedRef<IDetailCustomization> FACSSettingsDetails::MakeInstance()
{
	return MakeShareable(new FACSSettingsDetails);
}

void FACSSettingsDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DamageTypeCategory = DetailBuilder.EditCategory("Damage Type", FText::GetEmpty(), ECategoryPriority::Uncommon);

	ACSSettings = UACSSettings::Get();
	check(ACSSettings);

	DamageTypeEnum = StaticEnum<EDamageType>();
	check(DamageTypeEnum)

	TSharedPtr<IPropertyHandle> DamageTypesProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UACSSettings, DamageTypes));

	TSharedRef<FDamageTypeList> DamageTypeListCustomization = MakeShareable(new FDamageTypeList(ACSSettings, DamageTypeEnum, DamageTypesProperty));
	DamageTypeListCustomization->RefreshDamageTypeList();

	const FString DamageTypeDocLink = TEXT("AdvancedCombatSystem/Damage");
	TSharedPtr<SToolTip> DamageTypeTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("DamageType", "Edit damage type."), NULL, DamageTypeDocLink, TEXT("DamageType"));

	//Customize collision section

	DamageTypeCategory.AddCustomRow(LOCTEXT("FACSSettingsDetails_DamageType", "Damage Type"))
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ToolTip(DamageTypeTooltip)
		.AutoWrapText(true)
		.Text(LOCTEXT("DamageType_Menu_Description", "You can have up to 62 custom Damage Types for your project. \nOnce you name each type, they will show up as Damage type in the editor"))
	];

	DamageTypeCategory.AddCustomBuilder(DamageTypeListCustomization);








	IDetailCategoryBuilder& AmmoTypeCategory = DetailBuilder.EditCategory("Ammo Type", FText::GetEmpty(), ECategoryPriority::Uncommon);

	ACSSettings = UACSSettings::Get();
	check(ACSSettings);

	AmmoTypeEnum = StaticEnum<EAmmoType>();
	check(AmmoTypeEnum)

	TSharedPtr<IPropertyHandle> AmmoTypesProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UACSSettings, AmmoTypes));

	TSharedRef<FAmmoTypeList> AmmoTypeListCustomization = MakeShareable(new FAmmoTypeList(ACSSettings, AmmoTypeEnum, AmmoTypesProperty));
	AmmoTypeListCustomization->RefreshAmmoTypeList();

	const FString AmmoTypeDocLink = TEXT("AdvancedCombatSystem/Ammo");
	TSharedPtr<SToolTip> AmmoTypeTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("AmmoType", "Edit Ammo type."), NULL, AmmoTypeDocLink, TEXT("AmmoType"));

	//Customize collision section

	AmmoTypeCategory.AddCustomRow(LOCTEXT("FACSSettingsDetails_AmmoType", "Ammo Type"))
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ToolTip(AmmoTypeTooltip)
		.AutoWrapText(true)
		.Text(LOCTEXT("AmmoType_Menu_Description", "You can have up to 62 custom Ammo Types for your project. \nOnce you name each type, they will show up as Ammo type in the editor"))
	];

	AmmoTypeCategory.AddCustomBuilder(AmmoTypeListCustomization);










	IDetailCategoryBuilder& WeaponsCategory = DetailBuilder.EditCategory("Weapons Type", FText::GetEmpty(), ECategoryPriority::Uncommon);

	ACSSettings = UACSSettings::Get();
	check(ACSSettings);

	WeaponsEnum = StaticEnum<EWeapon>();
	check(WeaponsEnum)

	TSharedPtr<IPropertyHandle> WeaponsProperty = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UACSSettings, Weapons));

	TSharedRef<FWeaponsList> WeaponListCustomization = MakeShareable(new FWeaponsList(ACSSettings, WeaponsEnum, WeaponsProperty));
	WeaponListCustomization->RefreshWeaponsList();

	const FString WeaponDocLink = TEXT("AdvancedCombatSystem/Damage");
	TSharedPtr<SToolTip> WeaponTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("Weapon", "Edit damage type."), NULL, WeaponDocLink, TEXT("Weapon"));

	//Customize collision section

	WeaponsCategory.AddCustomRow(LOCTEXT("FACSSettingsDetails_Weapon", "Damage Type"))
	[
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.ToolTip(WeaponTooltip)
		.AutoWrapText(true)
		.Text(LOCTEXT("Weapon_Menu_Description", "You can have up to 62 custom Damage Types for your project. \nOnce you name each type, they will show up as Damage type in the editor"))
	];

	WeaponsCategory.AddCustomBuilder(WeaponListCustomization);

}


#undef LOCTEXT_NAMESPACE