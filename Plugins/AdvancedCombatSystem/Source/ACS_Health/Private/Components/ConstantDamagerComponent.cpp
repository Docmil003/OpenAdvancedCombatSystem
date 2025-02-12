//Copyright 2024 Kingsley Shyne Mattis Sogorb.All Rights Reserved.

#include "Components/ConstantDamagerComponent.h"
#include "ACSCoreHealth.h"
#include "Components/HealthComponent.h"

#define REMOVE_ELEMENT_WHEN(Name, Property, Param) DamageDataCollection.RemoveAll([&Param](const auto& Name) { return Property == Param;});

typedef UHealthComponent UHC;

UConstantDamagerComponent::UConstantDamagerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickInterval = 1;
}

#pragma region Core

void UConstantDamagerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Damage(DeltaTime);
}

void UConstantDamagerComponent::Damage(float DeltaTime)
{
	for (auto It = DamageDataCollection.CreateIterator(); It; ++It)
	{
		It->Damage.Damage *= DeltaTime;
		It->Damage.DeltaDamage = DeltaTime;
		
		if(It->SpecificTarget.IsNone()){
			UHC::SendDamageToOwner(this, It->Damage); //If targetless
		}
		else if(UHC* Hc = GetOwner()->GetComponentByClass<UHC>()){
			auto Specific =Hc->FindHealthDataByTagPair(It->SpecificTarget, true);
			
			//Both health and absorbption components must be valid
			if(Specific.Key && Specific.Value){
				double foo;
				Hc->DamageComp(*Specific.Key,*Specific.Value, It->Damage, foo);
			}
			else{
				UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("Couldn't find target %s so sending Damage normaly"), *It->SpecificTarget.ToString())
				UHC::SendDamageToOwner(this, It->Damage); //If invalid targets
			}
		}
	
		if(It->bUndefinedLifetime) continue;
	
		if(It->DamageLifetime -= DeltaTime <= 0){
			It.RemoveCurrent();
		}
	
	}

	if(DamageDataCollection.IsEmpty()){
		UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("%s has no more damage to send. Deactivatting"), *GetName());
		Deactivate();
	}
}
#pragma endregion

#pragma region GETTERS

void UConstantDamagerComponent::GetDamageTypes(TArray<TEnumAsByte<EDamageType>>& DamageTypes) const
{
	DamageTypes.Empty(DamageDataCollection.Num());
	for(const FDamageSendData& Data : DamageDataCollection)
		DamageTypes.AddUnique(Data.Damage.Type);
}

bool UConstantDamagerComponent::HasDamageType(TEnumAsByte<EDamageType> DamageType) const
{
	return DamageDataCollection.ContainsByPredicate([&DamageType](const FDamageSendData& Data){
		return Data.Damage.Type == DamageType;
	});
}

#pragma endregion

#pragma region Setters

void UConstantDamagerComponent::AddDamageData(const FDamageSendData& Data, bool bForce)
{
	if(bForce || !HasDamageType(Data.Damage.Type)){
		int32 Index = DamageDataCollection.Add(Data);
		FDamageInfo& Damage = DamageDataCollection[Index].Damage;
		
		if(Data.SelfCauser) Damage.Sender = GetOwner();
		if(!IsActive()) Activate();
		
	}
	else{
		UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("%s had already the damage %i. Skipping..."), *GetName(), Data.Damage.Type)
	}
}

void UConstantDamagerComponent::RemoveDamageByIndex(int32 Index)
{
	DamageDataCollection.RemoveAt(Index);
}

void UConstantDamagerComponent::RemoveDamageByType(TEnumAsByte<EDamageType> Type)
{
	REMOVE_ELEMENT_WHEN(A, A.Damage.Type, Type);
}

void UConstantDamagerComponent::RemoveDamageByTarget(const FName& Target)
{
	REMOVE_ELEMENT_WHEN(A, A.SpecificTarget, Target);
}

#pragma endregion