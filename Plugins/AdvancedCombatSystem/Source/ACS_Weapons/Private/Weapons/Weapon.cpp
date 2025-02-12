// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Weapons/Weapon.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Weapons/WeaponInitializationData.h"
#include "Bullets/BulletsPoolSystem.h"

#define CREATE_FUNCTION_DELEGATE(VarName, Class, Function, ...) FTimerDelegate VarName; VarName.BindUFunction(this, GET_FUNCTION_NAME_CHECKED(Class, Function), ##__VA_ARGS__);

#define DELAYED_LAMBDA(Delay, Lambda) FTimerHandle UnhandleTimer; GetWorld()->GetTimerManager().SetTimer(UnhandleTimer, Lambda, Delay, false);

using namespace EExecutionTiming;

// Sets default values
AWeapon::AWeapon() : Super()
{
	// Set this pawn to call Tick() every frame. You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	//Executions
	TriggerMode = ETriggerMode::Single;
	TriggerExecution = Begin;
	AmmoReloadExecution = End;
	UnJamExecution = End;

	AmmoType = AmmoType_Default;
	WeaponType = Weapon_Default;

	//Bools
	bReloadByMagazine = true;
	bUsesAmmo = false;
	bReloading = false;
	bIsTriggering = false;
	bCancelReload = false;
	bTriggerIfAmmoLessThanRequired = false;
	bLooseExtraAmmoOnReload = false;
	bCanJam = false;
	bIsInitialized = false;

	//Integers
	BurstIndex = 0;
	BurstRate = 1;
	AmmoLoosePerTrigger = 1;
	IterationReloadStepAmount = 1;
	MagazineCapability = 30;

	//Floats
	TriggerRate = 1;
	JamProbability = 0;

	InitializationWeaponDataClass = UWeaponInitializationData::StaticClass();
}

#pragma region Initialization

void AWeapon::Initiate_Implementation()
{

	if (InitializationWeaponData && !IsInitialized())
	{

		if (SlotsEnum)
		{
			TMap<uint8, uint8>& Comps = InitializationWeaponData->Components;
			for (int32 slot = Comps.Num(); slot < SlotsEnum->NumEnums() - 1; ++slot)
			{
				InitializationWeaponData->Components.Add(slot, 0);
			}
		}

		UpdateMods(InitializationWeaponData->Components);

		if (bUsesMagazine) {
			MagazinePtr.SetPtrDynamic(InitializationWeaponData->AmmoInMagazine);
		}
		else if (bUsesAmmo) {
			MagazinePtr = SourceAmmoPtr;
		}

		bJammed = InitializationWeaponData->bJammed;

		bIsInitialized = true;

		ApplyModificators();
	}
	else if(!InitializationWeaponData) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("%s can't initialize because it has no initialization data instance"), *GetName())
	}

}

void AWeapon::PreSaveWeaponData_Implementation()
{
	if (!InitializationWeaponData) 
	{
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("%s can't presave this weapon's data because it has no initialization data instance. Save request will return null"), *GetName())
		return;
	}

	UWeaponInitializationData& Data = *InitializationWeaponData;

	Data.AmmoInMagazine = GetAmmoInMagazine();
	Data.bJammed = IsJammed();
	Data.Components = Modificators;

}

void AWeapon::AttachToOwner() 
{
	AttachToComponent(GetOwnerMesh(), FAttachmentTransformRules::SnapToTargetIncludingScale, SocketAttachment); 
}

#pragma endregion

#pragma region Inherance

void AWeapon::SaveData()
{
	PreSaveWeaponData();
	OnSaveDataRequest.Broadcast(this, InitializationWeaponData);
}

void AWeapon::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITORONLY_DATA

	GenerateModificatorsGuide();
	
#endif

}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	OnTriggeredDelegate.AddDynamic(this, &AWeapon::OnTriggered);

	//Set default ammo source to magazine
	if(IsUsingMagazine()) {
		MagazinePtr.SetPtrDynamic(0);
	}

	if (!InitializationWeaponData) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("%s has no initialization data instance in begin play, creating a default"), *GetName())
		InitializationWeaponData = NewObject<UWeaponInitializationData>(InitializationWeaponDataClass);
		PreSaveWeaponData();
	}

	GetWorld()->GetTimerManager().SetTimerForNextTick([this]() {
		Initiate();
	});

}

void AWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	SaveData();
}

#pragma endregion

#pragma region Modifications

void AWeapon::ApplyModificators()
{
	for (int32 i = Modificators.Num(); i < ModsPerSlot.Num(); ++i)
		Modificators.Add(i);

	UpdateMods(Modificators);
}

void AWeapon::UpdateMod_Implementation(uint8 Slot, uint8 Modification)
{
	if (uint8* Mod = Modificators.Find(Slot)) *Mod = Modification;
	else Modificators.Add(Slot, Modification);
}

void AWeapon::UpdateMods(const TMap<uint8, uint8>& Update)
{
	for (const auto& i : Update) 
		UpdateMod(i.Key, i.Value);
}

#if WITH_EDITOR

void AWeapon::GenerateModificatorsGuide()
{
	if (SlotsEnum && SlotsEnum->GetMaxEnumValue() <= ModsPerSlot.Num())
	{
		ComponentInfo.Empty();

		for (int32 i = 0; i < SlotsEnum->GetMaxEnumValue(); ++i)
		{
			FString str = FString::Printf(TEXT("Modifications for slot %d (%s)"), i, *SlotsEnum->GetDisplayNameTextByIndex(i).ToString());
			const UEnum* CurrentSlot = ModsPerSlot[i];

			if (!CurrentSlot) continue;

			for (int32 j = 0; j < CurrentSlot->GetMaxEnumValue(); ++j)
			{
				str.Append(FString::Printf(TEXT("\n%dX%d: %s. \tDesc: %s"), i, j, 
					*CurrentSlot->GetDisplayNameTextByIndex(j).ToString(), *CurrentSlot->GetToolTipTextByIndex(j).ToString()
				));
			}

			ComponentInfo.Add(FText::FromString(str));
		}
	}
}

#endif

#pragma endregion

#pragma region Ammo

void AWeapon::SetUsesMagazine(bool bNewUseMagazine)
{
	if (bUsesMagazine == bNewUseMagazine) return;
	bUsesMagazine = bNewUseMagazine;

	if (bUsesAmmo && !bNewUseMagazine) {
		*SourceAmmoPtr += *MagazinePtr;
		MagazinePtr.Clear();
	}

	if(bNewUseMagazine) MagazinePtr.SetPtrDynamic(0);

}

void AWeapon::SetUsesAmmo(bool NewUsesAmmo)
{
	if (bUsesAmmo == NewUsesAmmo) return;

	if (NewUsesAmmo) 
		ResolverMagazinePointer();
	
	bUsesAmmo = NewUsesAmmo;
}

void AWeapon::SetAmmoType(TEnumAsByte<EAmmoType> NewAmmoType)
{
	if(AmmoType == NewAmmoType) return;

	const TEnumAsByte<EAmmoType> PrevAmmo = AmmoType;
	AmmoType = NewAmmoType;
	OnAmmoTypeChanged.Broadcast(GetAmmoInMagazine(), PrevAmmo, AmmoType);
}

#pragma endregion

#pragma region Trigger

void AWeapon::Trigger_Implementation(const FCharWeaponAnim& Anim)
{
	//Handle if try to trigger on reload (early return)
	if (bReloading) {

		//if reload by iterations cancel reload
		if (!bReloadByMagazine) {
			CancelReload();
		}
		return;
	}

	//If can NOT trigger return early
	if (!CanTrigger()) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Display, TEXT("%s can't trigger"), *GetName())
		return;
	}

	if (TriggerMode == ETriggerMode::Single) { //trigger and cooldown 
		TriggerLogic(Anim);
		GetWorld()->GetTimerManager().SetTimer(Cooldown, this, &AWeapon::EndTriggerCooldown, 1.0 / TriggerRate, false);
	}

	else if (TriggerMode == ETriggerMode::Burst) { //triggers, executes burst and internally handles the cooldown
		TriggerLogic(Anim);
		CREATE_FUNCTION_DELEGATE(Del, AWeapon, TriggerBurst, Anim);
		GetWorld()->GetTimerManager().SetTimer(Cooldown, Del, 1.0 / TriggerRate, true);
	}

	else if (TriggerMode == ETriggerMode::Automatic) { //triggers indefenitetly, internally handles the cooldown
		bIsTriggering = true;
		TriggerLogic(Anim);
		CREATE_FUNCTION_DELEGATE(Del, AWeapon, AutoTrigger, Anim);
		GetWorld()->GetTimerManager().SetTimer(Cooldown, Del, 1.0 / TriggerRate, true);
	}

}

void AWeapon::TriggerReleased_Implementation()
{
	//Says it to stop triggering if is automatic
	if (TriggerMode == ETriggerMode::Automatic) {
		bIsTriggering = false;
	}
}

void AWeapon::TriggerLogic_Implementation(const FCharWeaponAnim& Anim)
{
	//If can jam take a chance
	if (bCanJam) {

		RandomJam();

		//if trigger jammed call the delegate event and early return
		if (bJammed) {
			UE_LOG(AdvancedCombatSystem_Weapons_Log, Display, TEXT("%s just got jammed"), *GetName())
			return;
		}
	}
	
	const double duration = PlayAnimStruct(Anim);

	if (TriggerExecution == Begin) {
		OnTriggeredDelegate.Broadcast();
	}
	else if(TriggerExecution == End) {
		FTimerHandle UnhandleTimer; 
		GetWorld()->GetTimerManager().SetTimer(UnhandleTimer, [this]() { 
			OnTriggeredDelegate.Broadcast(); 
		}, duration, false);
	}

	if (bUsesMagazine) {
		*MagazinePtr = FMath::Max(*MagazinePtr - AmmoLoosePerTrigger, 0);
	}
}

bool AWeapon::CanTrigger_Implementation() const
{
	//is not jammed and Has enought ammo or does not require at all
	return !bJammed && HasEnoughAmmoToShot();
}

float AWeapon::GetRemainingCooldown() const 
{
	return GetWorld()->GetTimerManager().GetTimerRemaining(Cooldown);
}

#pragma endregion

#pragma region Reload

bool AWeapon::CanReload_Implementation() const
{
	return !bReloading && IsUsingMagazine() && GetAmmoInMagazine() < MagazineCapability && GetAmmoFromSource() > 0;
}

void AWeapon::ReloadComplete(const FCharWeaponAnim& Anim)
{

#if !NO_LOGGING
	if (!IsUsingMagazine())
	{
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to reload weapon that does not use ammo or Magazine"))
		return;
	}
#endif

	if (!IsUsingMagazine()) return;

	//Tells its gonna start reloading
	InitiateReload();

	//When anim ends calls to end reload
	FTimerHandle UnhandledTimer;
	GetWorld()->GetTimerManager().SetTimer(UnhandledTimer, this, &AWeapon::EndReload, PlayAnimStruct(Anim), false);
	
	//If should add ammo when begin
	if (AmmoReloadExecution == Begin) {
		ReloadLogic();
	}
}

void AWeapon::ReloadByIterations(const FCharWeaponAnimArrayForIterations& Sequence)
{

#if !NO_LOGGING
	if (!IsUsingMagazine())
	{
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to reload weapon that does not use ammo or Magazine"))
		return;
	}
#endif

	if (!IsUsingMagazine()) return;

	//Tells to start reloading
	InitiateReload();

	FTimerHandle UnhandledTimer;

	//When end start reloading anim, starts the loop reload
	GetWorld()->GetTimerManager().SetTimer(UnhandledTimer, [this, Sequence]() {
		ReloadIterationLoop(Sequence.Loop, Sequence.End);
	}, ReloadIterationStart(Sequence.Start), false);

}

float AWeapon::ReloadIterationStart(const FCharWeaponAnim& Anim)
{
	//Returns the anim duration
	return PlayAnimStruct(Anim);

}

void AWeapon::ReloadIterationLoop(const FCharWeaponAnim& Anim, const FCharWeaponAnim& EndAnim)
{
	//Update ammo at start if set as it
	if (AmmoReloadExecution == Begin) {
		ReloadLogic();
	}

	FTimerHandle UnhandledTimer;

	//After the anim ends check if should add the ammo, if it should end the reload or keep it going
	GetWorld()->GetTimerManager().SetTimer(UnhandledTimer, [this, Anim, EndAnim]() {

		/*Reload at the end of the animation*/
		if (AmmoReloadExecution == End) {
			ReloadLogic();
		}

		//If cancels reloading, or magazine is full or owner has no more ammo end the reload
		if (bCancelReload || MagazineCapability == GetAmmoInMagazine() || GetAmmoFromSource() <= 0) {
			ReloadIterationEnd(EndAnim);
		}
		else { //otherwise continue reloading
			ReloadIterationLoop(Anim, EndAnim);
		}

	}, PlayAnimStruct(Anim), false); //Plays anim and feeds the duration into the delay

}

void AWeapon::ReloadIterationEnd(const FCharWeaponAnim& Anim)
{
	FTimerHandle UnhandledTimer;

	//When stop reload iteration ends sets reloading and its cancelation to false
	GetWorld()->GetTimerManager().SetTimer(UnhandledTimer, [this]() {
		bReloading = bCancelReload = false;
	}, PlayAnimStruct(Anim), false); //executes the anim use the duration as delay
}

void AWeapon::InitiateReload()
{
	/*Sets reloading to true*/
	bReloading = true;
}

void AWeapon::EndReload()
{
	//Add ammo at end if set as it should
	if (AmmoReloadExecution == End) {
		ReloadLogic();
	}
	bReloading = false; //ends the reloading
}

void AWeapon::ReloadLogic()
{

	if (!(bUsesMagazine && MagazinePtr)) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("%s attempted to reload when does not use ammo or magazine"), *GetName())
		OnReloadFailedDelegate.Broadcast();
		return;
	}

	//If reloads auto calculates the ammo to add
	if (bReloadByMagazine) {
		ReloadLogicManual(
			FMath::Min(GetAmmoRequiredToFill(), GetAmmoFromSource())	//Min between ammo required and avaible
		);
	}
	else { //if reloads by iterations adds the ammo by step
		ReloadLogicManual(IterationReloadStepAmount);
	}
}

void AWeapon::ReloadLogicManual(int32 AddAmmo)
{

	if(!(bUsesMagazine && MagazinePtr)) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("%s attempted to reload when does not use ammo or magazine"), *GetName())
		OnReloadFailedDelegate.Broadcast();
		return;
	};

	if (bUsesAmmo) {
		int32& Source = *SourceAmmoPtr;

		if (Source - AddAmmo >= 0) {
			Source -= AddAmmo;
		}
		else return; //if does not have enough ammo exit
	}

	if (bLooseExtraAmmoOnReload && bReloadByMagazine) {
		*MagazinePtr = AddAmmo; // if loose ammo and reloads at once then ammo wil be replaced
	}
	else {
		*MagazinePtr += AddAmmo; // otherwise ammo will be added
	}
	
	OnReloadedDelegate.Broadcast();


}

#pragma endregion

#pragma region Jamming

void AWeapon::RandomJam()
{
	//Can't randomly unjamm
	SetJammed(UKismetMathLibrary::RandomBoolWithWeight(JamProbability) || bJammed);
}

void AWeapon::UnJam(const FCharWeaponAnim& Anim)
{
	//Gets the anim duration
	const float duration = PlayAnimStruct(Anim);

	//Handles when to set jammed to false, if at the start or end of the animation
	if (UnJamExecution == Begin) { 
		SetJammed(false);
	}
	else if (UnJamExecution == End) {
		FTimerHandle UnHandle;
		GetWorld()->GetTimerManager().SetTimer(UnHandle, [this]() {
			SetJammed(false);
		}, duration, false); //Uses the anim duration to know when ends
	}
}

void AWeapon::SetJammed(const bool NewJammed)
{
	if(NewJammed!=bJammed)
	{
		bJammed = NewJammed;
		(bJammed ? OnJammedDelegate : OnUnJammedDelegate).Broadcast();
	}
}

#pragma endregion

#pragma region Magazine

void AWeapon::ResolverMagazinePointer()
{
	if (bUsesMagazine) {
		EnsurePointer(MagazinePtr, 0);
	}
	else if (bUsesAmmo) {
		MagazinePtr = SourceAmmoPtr;
	}
}

void AWeapon::SetMagazineCapability(int32 NewCapability)
{
	if (!IsUsingMagazine()) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to set magazine capability when there is no magazine in use"))
		return; //If not uses magazine no fix shall be done
	}
	
	if (NewCapability <= 0) {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to set magazine capability to a non-positive number"))
		return;
	}

	//Represents the ammount of ammo that now can't fit
	const int32 Diff = GetAmmoInMagazine() - NewCapability;

	MagazineCapability = NewCapability;
	
	EnsurePointer(MagazinePtr, 0);
	*MagazinePtr -= FMath::Max(0, Diff);

	if (bUsesAmmo && Diff > 0) { //The ammo exceeded will be returned to source
		*SourceAmmoPtr += Diff;
	}
	
}

void AWeapon::SetAmmoInMagazine(const int32 NewAmmo)
{
	if (bUsesAmmo && IsUsingMagazine() && MagazinePtr) 
		*MagazinePtr = NewAmmo;
}

bool AWeapon::SetSourceForAmmoFromMap(TMap<TEnumAsByte<EAmmoType>, int32>& Map)
{
	if (int32* Container = Map.Find(AmmoType)) {
		SourceAmmoPtr.SetPtrStatic(*Container); //Must share the value
		ResolverMagazinePointer();
		return true;
	}
	UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to set the source ammo, but the map variable does not have the AmmoType of %s (%i)"), *GetName(), AmmoType)
	return false;
}

#pragma endregion

#pragma region Utils

UMeshComponent* AWeapon::GetWeaponMesh_Implementation() const 
{ 
	return GetComponentByClass<USkeletalMeshComponent>(); 
};

float AWeapon::PlayAnimStruct_Implementation(const FCharWeaponAnim& AnimToPlay, const bool ReturnCharacterAnimDuration)
{
	//Plays weapon and character anim
	float WeapAnimDuration = 0;

	if (USkeletalMeshComponent* sk = Cast<USkeletalMeshComponent>(GetWeaponMesh())) {
		if (AnimToPlay.Weapon)
		{
			sk->SetPlayRate(AnimToPlay.PlayRate);
			sk->PlayAnimation(AnimToPlay.Weapon, false);
			WeapAnimDuration = AnimToPlay.Weapon->GetPlayLength() / AnimToPlay.PlayRate;
		}
	}
	if (ACharacter* character = GetOwnerCharacter()) {
		if(ReturnCharacterAnimDuration && AnimToPlay.Character)
			return character->PlayAnimMontage(AnimToPlay.Character, AnimToPlay.PlayRate); //Char anim length is the used as anim duration
	}

	return WeapAnimDuration;
}

void AWeapon::PlayAnimSynchronous(ACharacter* Character, AWeapon* Weapon,
	const FCharWeaponAnim& AnimToPlay, float& CharacterAnimDuration, float& WeaponAnimDuration)
{

	if (USkeletalMeshComponent* SK = Cast<USkeletalMeshComponent>(Weapon->GetWeaponMesh())) {
		if(AnimToPlay.Weapon)
		{
			SK->PlayAnimation(AnimToPlay.Weapon, false);
			WeaponAnimDuration = AnimToPlay.Weapon->GetPlayLength() / AnimToPlay.PlayRate;
		}
	}

	CharacterAnimDuration = Character->PlayAnimMontage(AnimToPlay.Character); //Char anim length is the used as anim duration

}

#pragma endregion

#pragma region TriggerBind

void AWeapon::TriggerBurst(const FCharWeaponAnim& Anim)
{
	//If can trigger and burst index still in range
	if (CanTrigger() && ++BurstIndex < BurstRate) {
		TriggerLogic(Anim);
	}
	else { //otherwise stop burst
		BurstIndex = 0;
		EndTriggerCooldown();
	}
}

void AWeapon::AutoTrigger(const FCharWeaponAnim& Anim)
{
	//If can trigger and trigger is pressed
	if (CanTrigger() && bIsTriggering) {
		++BurstIndex;
		TriggerLogic(Anim);
	}
	else { //otherwise end trigger
		BurstIndex = 0;
		EndTriggerCooldown();
	}
}

void AWeapon::EndTriggerCooldown()
{
	GetWorld()->GetTimerManager().ClearTimer(Cooldown);
}

#pragma endregion
