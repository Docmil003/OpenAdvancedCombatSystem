// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Components/HealthComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "ACS_CoreElements.h"

DEFINE_LOG_CATEGORY(AdvancedCombatSystem_Health_Log);

//Implements a filter by predicate. Must pass the variable name and the access to the condition (Element is const auto A)
#define HEALTH_FILTER(VarName, HealthAccess) VarName = VarName.FilterByPredicate([](const auto& A) { return HealthAccess; });

//The macro binds the delegate to the function only if not existed already
#define BIND_EVENT(Event) if(!CompToBind->Event ## Delegate.IsBoundToObject(this)) CompToBind->Event ## Delegate.AddUObject(this, &UHealthComponent::Event);

UHealthComponent::UHealthComponent()
{
	// There's no need in tick
	PrimaryComponentTick.bCanEverTick = false;

	ArmordefenseMethod = EArmordefenseMethod::Mostdefense;
	bIncludeMainHealthOnDamage = false;
	bOnlyOneArmorDamageAtTime = false;

	Health.Key.bMainHealth = true;

	Health.Key = InitialHealth;
	Health.Value = InitialDefense;

}

#pragma region Hinerance

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	SetUpBindings(this);
	Health.Key = InitialHealth;
	Health.Value = InitialDefense;
}

#pragma endregion

#pragma region Health

void UHealthComponent::SetHealthComponent(const FHealth& NewHealth)
{
	Health.Key = NewHealth;
	Health.Key.OwnerHealthComp = this;
	Health.Key.bMainHealth = true;
}

void UHealthComponent::SetHealthDefenseComponent(const FDefenseProperties& NewDefense)
{
	Health.Value = NewDefense;
}

void UHealthComponent::SetHealth(double NewHealth)
{
	*Health.Key.Health = NewHealth;
}

void UHealthComponent::SetMaxHealth(double NewMaxHealth)
{
	*Health.Key.MaxHealth = NewMaxHealth;
}

void UHealthComponent::SetHealthPair(const FHealthPair& NewHealthPair)
{
	Health = NewHealthPair;
	Health.Key.OwnerHealthComp = this;
	Health.Key.bMainHealth = true;
}

#pragma endregion

#pragma region Armor

void UHealthComponent::SetArmor(const TMap<FHealth, FDefenseProperties>& NewArmor)
{
	Armor = NewArmor;
	for (auto Elem : Armor) {
		Elem.Key.OwnerHealthComp = this;
	}
}

void UHealthComponent::SetArmorHealth(double NewArmor, const int32 Index)
{
	*GetArmorComponent(Index).Health = NewArmor;
}

void UHealthComponent::SetMaxArmorHealth(double NewMaxArmor, const int32 Index)
{
	*GetArmorComponent(Index).MaxHealth = NewMaxArmor;
}

void UHealthComponent::SetArmorComponent(const FHealth& NewHealth, const int32 Index)
{
	FHealth& HealthRef = const_cast<FHealth&>(GetArmorComponent(Index));
	HealthRef = NewHealth;
	HealthRef.OwnerHealthComp = this;
}

void UHealthComponent::SetArmorsDefenseComponent(const FDefenseProperties& Newdefense, const int32 Index)
{
	const_cast<FDefenseProperties&>(GetArmorsDefenseComponent(Index)) = Newdefense;
}

void UHealthComponent::SetArmorPair(const FHealthPair& NewArmorPair, const int32 Index)
{
	SetArmorComponent(NewArmorPair.Key, Index);
	SetArmorsDefenseComponent(NewArmorPair.Value, Index);
}

#pragma endregion

#pragma region Delegate Functions

void UHealthComponent::OnDamaged(const AActor* Actor, const FDamageInfo& Damage)
{
	OnDamagedBP(Actor, Damage);
	OnDamaged_BP.Broadcast(Actor, Damage);
}

void UHealthComponent::OnDamagedHealth(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage)
{
	OnDamagedHealthBP(Actor, HealthComp, Damage);
}

void UHealthComponent::OnDamagedArmor(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage)
{
	OnDamagedArmorBP(Actor, HealthComp, Damage);
}

void UHealthComponent::OnKilled(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage)
{
	OnKilled_BP.Broadcast(Actor, HealthComp, Damage);
	OnKilledBP(Actor, HealthComp, Damage);
}

void UHealthComponent::OnArmorDestroyed(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage)
{
	OnArmorDestroyedBP(Actor, HealthComp, Damage);
	OnArmorDestroyed_BP.Broadcast(Actor, HealthComp, Damage);
}

void UHealthComponent::OnHealed(const AActor* Actor, const FHealInfo& Heal)
{
	OnHealedBP(Actor, Heal);
	OnHealed_BP.Broadcast(Actor, Heal);
}

void UHealthComponent::OnHealthHeal(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal)
{
	OnHealthHealBP(Actor, HealthComp, Heal);
}

void UHealthComponent::OnArmorHeal(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal)
{
	OnArmorHealBP(Actor, HealthComp, Heal);
}

void UHealthComponent::OnRevived(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal)
{
	OnRevivedBP(Actor, HealthComp, Heal);
	OnRevived_BP.Broadcast(Actor, HealthComp, Heal);
}

void UHealthComponent::OnRestoredArmor(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal)
{
	OnRestoredArmorBP(Actor, HealthComp, Heal);
	OnArmorRestored_BP.Broadcast(Actor, HealthComp, Heal);
}

#pragma endregion

#pragma region Damage

void UHealthComponent::Damage(const AActor* Actor, const FDamageInfo& Damage)
{
	OnDamagedDelegate.Broadcast(Actor, Damage);
	ProcessDamage(Damage);
}

double UHealthComponent::ProcessDamage(const FDamageInfo& DamageInfo)
{
	
	double AccumulatedDamage = 0;
	double RemainingDamage = DamageInfo.Damage;



	FHealthPtrMap ValidArmors;
	GetArmorPtr(ValidArmors);

	if(bIncludeMainHealthOnDamage){
		ValidArmors.Add(&Health.Key, &Health.Value);
	}

	if (bOnlyOneArmorDamageAtTime) {
		AccumulatedDamage = HandleDamageDefense(ValidArmors, DamageInfo, RemainingDamage);
	} else {
		while (RemainingDamage && IsAnyAlive(ValidArmors)) {
			AccumulatedDamage += HandleDamageDefense(ValidArmors, DamageInfo, RemainingDamage);
		}
	}

	//If ammo still remains reduce it from the main health
	if (RemainingDamage > 0) {
		FDamageInfo RemainingDamageInfo = FDamageInfo(DamageInfo);
		RemainingDamageInfo.Damage = RemainingDamage;

		DamageComp(Health.Key, Health.Value, RemainingDamageInfo, RemainingDamage);
	}

	return AccumulatedDamage;

}

double UHealthComponent::DamageComp(FHealth& HealthComp, const FDefenseProperties& Absorb, const FDamageInfo& DamageInfo, double& RemainingDamage, double DamageScale)
{
	//Allow an easy access
	double& HealthRef = *HealthComp.Health;

	const double DamageAbsorbed = DamageInfo.Damage * CalculateAbsorbption(Absorb, DamageInfo);

	RemainingDamage = DamageInfo.Damage - DamageAbsorbed;

	//It can have a default value, so should be scaled by the calculated scale
	DamageScale *= GetDamageScale(Absorb, DamageInfo);

	const double DamageReceived = DamageAbsorbed * DamageScale;

	HealthRef -= DamageReceived;

	auto Delegate = HealthComp.bMainHealth ? OnDamagedHealthDelegate : OnDamagedArmorDelegate;
	Delegate.Broadcast(UCoreHelperFunctions::GetDamagedActor(DamageInfo), HealthComp, DamageInfo);

	if (HealthRef <= 0) {

		//Invert the damage scale from the below 0 damage health
		RemainingDamage += -HealthRef / DamageScale;

		HealthRef = 0;

		Delegate = HealthComp.bMainHealth ? OnKilledDelegate : OnArmorDestroyedDelegate;

		Delegate.Broadcast(UCoreHelperFunctions::GetDamagedActor(DamageInfo), HealthComp, DamageInfo);
	}

	return HealthRef;
}

double UHealthComponent::CalculateAbsorbption(const FDefenseProperties& Defense, const FDamageInfo& Damage)
{
	double AbsorbScale = 1;
	const USkeletalMeshComponent* const Mesh = Cast<USkeletalMeshComponent>(Damage.HitComponent.Get());
	FName Bone = Damage.Bone;
	bool AppliedAnyScale = false;

	//If the bone is not in the list, search for the closest parent bone
	if (Mesh && !Defense.DamageBoneAbsorbption.IsEmpty())
	{
		if (!Defense.DamageBoneAbsorbption.Contains(Bone))
		{
			TArray<FName> List;
			Defense.DamageBoneAbsorbption.GenerateKeyArray(List);
			UCoreHelperFunctions::GetClosestParentBoneInArray(Mesh, Bone, List);
		}

		//If the bone is recognized, scale the base scale
		if (const double* BoneScale = Defense.DamageBoneAbsorbption.Find(Bone))
		{
			AbsorbScale *= *BoneScale / 100.0;
			AppliedAnyScale = true;
		}
	}
#if !NO_LOGGING
	else if(!Defense.DamageBoneAbsorbption.IsEmpty() && Damage.HitComponent) {
		UE_LOG(AdvancedCombatSystem_Health_Log, Error, TEXT("%s has absorbption based on bone, but has no mesh"), *Damage.HitComponent->GetName())
	}
#endif

	//If the damage type is recognized, scale the base scale
	if (const double* TypeScale = Defense.DamageTypeAbsorbtion.Find(Damage.Type)) {
		AbsorbScale *= *TypeScale / 100.0;
		AppliedAnyScale = true;
	}

	//If could not use any specific scale use the defaults
	if (!AppliedAnyScale)
	{
		AbsorbScale *= Defense.DefaultDamageAbsorbption / 100.0;
	}

	return AbsorbScale;

}

double UHealthComponent::GetDamageScale(const FDefenseProperties& Defense, const FDamageInfo& Damage, double Scale)
{
	const USkeletalMeshComponent* const Mesh = Cast<USkeletalMeshComponent>(Damage.HitComponent);
	FName Bone = Damage.Bone;
	bool AppliedAnyScale = false;

	if (Mesh && !Defense.ScaleDamagePerBone.IsEmpty())
	{
		//If the bone is not in the list, search for the closest parent bone
		if (!Defense.ScaleDamagePerBone.Contains(Bone))
		{
			TArray<FName> List;
			Defense.ScaleDamagePerBone.GenerateKeyArray(List);

			UCoreHelperFunctions::GetClosestParentBoneInArray(Mesh, Bone, List);

		}

		//If the bone is recognized, scale the base scale
		if (const double* BoneScale = Defense.ScaleDamagePerBone.Find(Bone))
		{
			Scale *= *BoneScale / 100.0;
			AppliedAnyScale = true;
		}
	}
#if !NO_LOGGING
	else if (!Defense.ScaleDamagePerBone.IsEmpty() && Damage.HitComponent) {
		UE_LOG(AdvancedCombatSystem_Health_Log, Error, TEXT("%s has absorbption based on bone, but has no mesh"), *Damage.HitComponent->GetName())
	}
#endif

	//If the damage type is recognized, scale the base scale
	if (const double* TypeScale = Defense.ScaleDamagePerType.Find(Damage.Type)) {
		Scale *= *TypeScale / 100.0;
		AppliedAnyScale = true;
	}

	//If could not use any specific scale use the defaults
	if (!AppliedAnyScale)
	{
		Scale *= Defense.DefaultScaleDamage / 100.0;
	}

	return Scale;
}

bool UHealthComponent::SendDamageToActorStruct(const FActorDamageInfo& Info)
{
	return SendDamageToActor(Info.Victim, Info.Damage);
}

bool UHealthComponent::SendDamageToActor(const AActor* Target, const FDamageInfo& Damage)
{//If actor is valid and has HealthComp
	if (Target) {
		if (UHealthComponent* Hc = Target->GetComponentByClass<UHealthComponent>())
		{
			Hc->Damage(Target, Damage);
			return true;
		}
	}
	return false;
}

bool UHealthComponent::SendDamageToActorsStruct(const TArray<FActorDamageInfo>& List, const bool bSkipNoDamage)
{
	//The predicate calls the individual function, and returns an array of failed actors. If is empty then there's no failed actor. Can't use contain cuz would break the loop when true
	return List.FilterByPredicate([bSkipNoDamage](const FActorDamageInfo& A) { return (!bSkipNoDamage || A.Damage.Damage > 0) && !SendDamageToActor(A.Victim, A.Damage); }).IsEmpty();
}

bool UHealthComponent::SendDamageToActors(const TArray<AActor*>& Target, const FDamageInfo& Damage, const bool bSkipNoDamage)
{
	//The predicate calls the individual function, and returns an array of failed actors. If is empty then there's no failed actor. Can't use contain cuz would break the loop when true
	return Target.FilterByPredicate([Damage, bSkipNoDamage](const AActor* const& A) { return (!bSkipNoDamage || Damage.Damage > 0) && !SendDamageToActor(A, Damage); }).IsEmpty();
}

#pragma endregion

#pragma region Heal

void UHealthComponent::Heal(const AActor* Actor, const FHealInfo& Heal)
{
	FHealInfo HealData(Heal);
	OnHealedDelegate.Broadcast(Actor, HealData);
	ProcessHeal(HealData);
}

void UHealthComponent::ProcessHeal(FHealInfo& HealthInfo)
{
	//Find the component to apply
	if (FHealth* Target = FindHealthDataByTag(HealthInfo.TargetTag)) {
		BuildAndApplyHealth(*Target, HealthInfo);

		//Handle health over limit
		HandleOverHealed(*Target);
	}
}

void UHealthComponent::BuildAndApplyHealth(FHealth& HealthComp, FHealInfo& Info)
{
	//Heal can be set in a strict value or relative to max health
	if (Info.bBruteHealth) {
		Info.HealPercentage = Info.Health / *HealthComp.MaxHealth * 100;
	}
	else {
		Info.Health = *HealthComp.MaxHealth * Info.HealPercentage / 100;
	}

	//Apply raw value
	*HealthComp.Health += Info.Health;
}

void UHealthComponent::BeginDecreaseExceededHealth(FHealth& Target)
{
	//Initiates a overdose handle with the data of the Target health comp
	GetWorld()->GetTimerManager().SetTimer(Target.OverdoseHandle, [&Target, this]() {

		if (!Target.IsOverdosed()) {
			GetWorld()->GetTimerManager().ClearTimer(Target.OverdoseHandle);
		}

		//Min between tick decrease and diference to avoid overdose
		const double Decrement = FMath::Min(Target.OverdoseTick * Target.OverdoseDecrease, *Target.Health - *Target.MaxHealth);
		*Target.Health -= FMath::Max(Decrement, 0);

	}, Target.OverdoseTick, true, Target.OverdoseReductionDelay);

}

void UHealthComponent::HandleOverHealed(FHealth& Element)
{
	if (!Element.IsOverdosed()) return;

	if (Element.bCanOverdose) {

		//If health overdose runneth over will be clamped to the limit if has
		if (Element.OverdoseLimit > 0) {
			*Element.Health = FMath::Min(*Element.Health, *Element.MaxHealth + Element.OverdoseLimit);
		}

		//If already was over limit and bReestarOverdoseWhenReRunneth, pause the health reduction again
		if (Element.bDelayWhenRehealed && Element.OverdoseHandle.IsValid()) {
			GetWorld()->GetTimerManager().ClearTimer(Element.OverdoseHandle);
		}

		//If there was no overdose previously then start it
		if (!Element.OverdoseHandle.IsValid()) {
			BeginDecreaseExceededHealth(Element);
		}
	}
	else {
		Element.Health = Element.MaxHealth;
	}
}

bool UHealthComponent::SendHealthToActor(const AActor* Target, const FHealInfo& Heal)
{//If actor is valid and has HealthComp
	if (Target) {
		if (UHealthComponent* Hc = Target->GetComponentByClass<UHealthComponent>())
		{
			Hc->Heal(Target, Heal);
			return true;
		}
	}
	return false;
}

bool UHealthComponent::SendHealthToActors(const TArray<AActor*>& Target, const FHealInfo& Heal)
{
	//The predicate calls the individual function, and returns an array of failed actors. If is empty then there's no failed actor
	return Target.FilterByPredicate([Heal](const AActor* const& A) { return !SendHealthToActor(A, Heal); }).IsEmpty();
}

#pragma endregion

#pragma region BP ONLY

bool UHealthComponent::FindHealthByTag_BP(const FName& TargetTag, FHealth& HealthComp)
{
	if (FHealth* Found = FindHealthDataByTag(TargetTag, true)) {
		HealthComp = *Found;
		return true;
	} return false;
}

void UHealthComponent::SortByDefense_BP(TMap<FHealth, FDefenseProperties>& Map, const FDamageInfo& Damage, bool bIncludeMain) const
{
	Map = Armor;
	if(bIncludeMain) Map.Add(Health);
	SortByDefense(Map, Damage);
}

void UHealthComponent::FilterAlive_BP(TArray<FHealth>& Array, bool bIncludeMain) const
{
	GetAllHealths(Array, bIncludeMain);
	FilterAlive(Array);
}

bool UHealthComponent::IsAnyAlive_BP(bool bIncludeMain) const
{
	//any armor not broken, AND, (IF includes main, must be alive too)
	return IsAnyAlive(Armor) && (!bIncludeMain || Health.Key);
}

#pragma endregion

#pragma region Miscellaneous

void UHealthComponent::DivideDamageOnElements(const FHealthPtrMap& Components, const FDamageInfo& DamageInfo, double& RemainingDamage, double& DamageGathered)
{
	const double DamageScale = 1.0 / Components.Num();	//It applies to the default damage scale
	for (auto It = Components.CreateConstIterator(); It; ++It) {
		DamageGathered += *It.Key()->Health - DamageComp(*It.Key(), *It.Value(), DamageInfo, RemainingDamage, DamageScale);
	}
}

FHealth* UHealthComponent::DamageFirst(const FHealthPtrMap& Components, const FDamageInfo& DamageInfo,
	double& RemainingDamage, double& DamageGathered)
{
	if (Components.IsEmpty()) return nullptr;

	//Use an iterator to have better access to properties, since maps don't have operator[int]
	const auto It = Components.CreateConstIterator();

	DamageGathered += *It->Key->Health - DamageComp(*It->Key, *It->Value, DamageInfo, RemainingDamage);

	return It->Key;
}

double UHealthComponent::HandleDamageDefense(FHealthPtrMap& Components, const FDamageInfo& DamageInfo, double& UnabsorbedDamage)
{
	double AccumulatedDamage = 0;

	//Handle defense method
	switch (ArmordefenseMethod)
	{

	case EArmordefenseMethod::DivideThroughAll: //Damages each piece of armor and adds that damage to the baked damage, that will return the total scaled damage received
		DivideDamageOnElements(Components, DamageInfo, UnabsorbedDamage, AccumulatedDamage);
		Components.Empty();
		break;

	case EArmordefenseMethod::Mostdefense:		//Sorts and damages the first. Does not use break to optimize and reuse FirstFound
		SortByDefense(Components, DamageInfo);

	case EArmordefenseMethod::FirstFound:		//Damages the first component of the array as it removes it
		Components.Remove(DamageFirst(Components, DamageInfo, UnabsorbedDamage, AccumulatedDamage));
		break;

	default:
		UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("Damage has been set to handle by an undefined method (%i)"), ArmordefenseMethod)
	}

	return AccumulatedDamage;

}

#pragma endregion

#pragma region Health Sharing

bool UHealthComponent::ShareHealth(const FName& TagName, const TArray<UHealthComponent*>& ShareWith, TEnumAsByte<enum EHealthCombinationMethod::Type> CombinationMethod)
{
	if (const FHealth* HealthToShare = FindHealthDataByTag(TagName)) {

		bool AnyValid = false;

		//Components that were actually binded
		TArray<UHealthComponent*> Binded;
		Binded.Add(this);
		ClearBindings(ShareWith);

		//Information for the combined health
		TArray<FVector2D> SharedHealths;
		SharedHealths.Add(FVector2D(*HealthToShare->Health, *HealthToShare->MaxHealth));
		
		for (UHealthComponent* OtherComp : ShareWith) {
			if (!OtherComp) continue;

			OtherComp->ClearBindings(this); //Binds should reset

			if (FHealth* Receiver = OtherComp->FindHealthDataByTag(TagName)) {

				SharedHealths.Add(FVector2D(*Health.Key.Health, *Health.Key.MaxHealth));

				//Other binded to this
				Receiver->Health = HealthToShare->Health;
				Receiver->MaxHealth = HealthToShare->MaxHealth;

				//Adds to the array the bind
				Binded.Add(OtherComp);
				SharedHealths.Emplace(Health.Key);

				AnyValid = true;
			}
			else {
				UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("%s has not a Health tagged with %s so it can not be shared"), *OtherComp->GetName(), *TagName.ToString())
			}
		}

		//Binds all elements to the rest. Similar concept to "fully connected mesh" topology
		for(auto It = Binded.CreateConstIterator(); It; ++It)
		{
			//Sets the binding for all components after current iterator.
			for (int32 i = It.GetIndex(); i < Binded.Num(); ++i) {
				Binded[i]->SetUpBindings(*It, HealthToShare->bMainHealth, i != It.GetIndex());
			}
		}

		//Calculate and apply the new health and max health
		FVector2D HealthCalculated = CombinationMethodCalculator(SharedHealths, CombinationMethod);

		*HealthToShare->Health = HealthCalculated.X;
		*HealthToShare->MaxHealth = HealthCalculated.Y;

		return AnyValid;

	}
	UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("%s has not a Health tagged with %s so it can not be shared"), *GetName(), *TagName.ToString())
	return false;
}

FVector2D UHealthComponent::CombinationMethodCalculator(const TArray<FVector2D>& Healths, const EHealthCombinationMethod::Type CombinationMethod)
{
	auto It = Healths.CreateConstIterator();
	FVector2D Result(0, 0);

	if (!It) return Result;

	switch (CombinationMethod)
	{
	case EHealthCombinationMethod::Combine:
		do {
			Result += *It;
		} while (++It); break;

	case EHealthCombinationMethod::Average:
		do {
			Result += (*It / double(Healths.Num()));
		} while (++It); break;

	case EHealthCombinationMethod::Max:
		do {
			Result.X = FMath::Max(Result.X, It->X);
			Result.Y = FMath::Max(Result.Y, It->Y);
		} while (++It); break;
	
	case EHealthCombinationMethod::Min:
		do {
			Result.X = FMath::Min(Result.X, It->X);
			Result.Y = FMath::Min(Result.Y, It->Y);
		} while (++It); break;

	default:
		UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("Health share calculation has been set to unknown method (%i)"), CombinationMethod)
	}

	return Result;
}

void UHealthComponent::SetUpBindings(UHealthComponent* CompToBind, bool IsHealth, bool BindToComp)
{
	
	//These are both health and armor properies
	BIND_EVENT(OnDamaged);
	BIND_EVENT(OnHealed);

	if (IsHealth) {	//these are health exclusive
		BIND_EVENT(OnKilled);
		BIND_EVENT(OnRevived);
		BIND_EVENT(OnDamagedHealth)
	}
	else {	//These are armor exclusive
		BIND_EVENT(OnArmorDestroyed);
		BIND_EVENT(OnRestoredArmor);
		BIND_EVENT(OnDamagedArmor);
	}

	if (BindToComp && CompToBind != this) {	//If specified the component that has bind to will be binded to this
		CompToBind->SetUpBindings(this, IsHealth);
	}

}

void UHealthComponent::SetUpBindings(UHealthComponent* CompToBind)
{
	//Will bind both Main health and armor
	SetUpBindings(CompToBind, true);
	SetUpBindings(CompToBind, false);
}

void UHealthComponent::ClearBindings(UHealthComponent* CompsToUnbind)
{
	OnDamagedDelegate.RemoveAll(CompsToUnbind);
	OnHealedDelegate.RemoveAll(CompsToUnbind);
	OnKilledDelegate.RemoveAll(CompsToUnbind);
	OnArmorDestroyedDelegate.RemoveAll(CompsToUnbind);
	OnRevivedDelegate.RemoveAll(CompsToUnbind);
	OnRestoredArmorDelegate.RemoveAll(CompsToUnbind);
}

void UHealthComponent::ClearBindings(const TArray<UHealthComponent*>& CompsToUnbind)
{
	for (const auto& Comp : CompsToUnbind) ClearBindings(Comp);
}

#pragma endregion

#pragma region Filter And Sort

void UHealthComponent::SortByDefense(FHealthPtrMap& Map, const FDamageInfo& Damage)
{
	Map.ValueSort([&Damage](const FDefenseProperties& A, const FDefenseProperties& B) {
		return GetDamageScale(A, Damage) < GetDamageScale(B, Damage);
	});
}

void UHealthComponent::SortByDefense(FHealthMap& Map, const FDamageInfo& Damage)
{
	Map.ValueSort([&Damage](const FDefenseProperties& A, const FDefenseProperties& B) {
		return GetDamageScale(A, Damage) < GetDamageScale(B, Damage);
	});
}

void UHealthComponent::FilterAlive(TArray<FHealth*>& Array)
{
	HEALTH_FILTER(Array, *A)
}

void UHealthComponent::FilterAlive(TArray<FHealth>& Array)
{
	HEALTH_FILTER(Array, A)
}

void UHealthComponent::FilterAlive(FHealthPtrMap& Map)
{
	HEALTH_FILTER(Map, *A.Key->Health)
}

bool UHealthComponent::IsAnyAlive(const TArray<FHealth*> Array)
{
	return Array.ContainsByPredicate([](const FHealth* A) { return *A; });
}

bool UHealthComponent::IsAnyAlive(const FHealthPtrMap& Map)
{
	for (const auto& Pair : Map) {
		if (*Pair.Key) return true;
	}
	return false;
}

bool UHealthComponent::IsAnyAlive(const FHealthMap& Map)
{
	for (const auto& Pair : Map) {
		if (Pair.Key) return true; //Only needs one
	}
	return false;
}

#pragma endregion

#pragma region Utils

USkeletalMeshComponent* UHealthComponent::GetMainMesh_Implementation() const
{
	if (ACharacter* Char = Cast<ACharacter>(GetOwner())) return Char->GetMesh();
	return GetOwner()->GetComponentByClass<USkeletalMeshComponent>();
}

void UHealthComponent::GetAllTags(TArray<FName>& Tags) const
{
	Tags.Empty(Armor.Num() + 1);	//Prepares to add the correct number of elements

	Tags.Add(Health.Key.Tag);
	for (const FHealthPair& Piece : Armor)
		Tags.Add(Piece.Key.Tag);
}

void UHealthComponent::GetAllHealths(TArray<FHealth>& Healths, bool bIncludeMain) const 
{
	Armor.GenerateKeyArray(Healths);
	if (bIncludeMain) Healths.Add(Health.Key);
}

FHealth* UHealthComponent::FindHealthDataByTag(const FName& TargetTag, const bool ReturnMainWhenNotFound)
{
	return FindHealthDataByTagPair(TargetTag, ReturnMainWhenNotFound).Key;
}

FHealthPtrPair UHealthComponent::FindHealthDataByTagPair(const FName& TargetTag, const bool ReturnMainWhenNotFound)
{
	FHealthPtrPair MainPair(&Health.Key, &Health.Value); //Save to reuse if not found
	if (Health.Key == TargetTag) return MainPair;

	for (FHealthPair& Pair : Armor)
	{
		if (Pair.Key == TargetTag) return FHealthPtrPair(&Pair.Key, &Pair.Value);
	}
	
	UE_LOG(AdvancedCombatSystem_Health_Log, Warning, TEXT("%s has not a Health tagged with %s so returning %s"), *GetName(), *TargetTag.ToString(), ReturnMainWhenNotFound ? *MainPair.Key->Tag.ToString() : TEXT("Null"))
	//if forces to return will return main health
	return ReturnMainWhenNotFound ? MainPair : FHealthPtrPair();
}

const FHealthPair& UHealthComponent::GetArmorPair(const int32 Index) const
{
	auto It = Armor.CreateConstIterator();
	for (int32 i = 0; i < Index; i++) ++It;
	return *It;
}

void UHealthComponent::GetArmorPtr(FHealthPtrMap& Map)
{
	Map.Empty(Armor.Num());
	for (FHealthPair& pair : Armor) {
		Map.Add(&pair.Key, &pair.Value);
	}
}

#pragma endregion
