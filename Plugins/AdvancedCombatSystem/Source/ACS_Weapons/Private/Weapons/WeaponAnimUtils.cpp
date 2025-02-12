// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Weapons/WeaponAnimUtils.h"

#include "Weapons/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Weapons/SubWeapons/FireWeapon.h"
#include "Weapons/SubWeapons/MeleeWeapon.h"

#pragma region UtilsInline

inline bool HasWeaponInterface(const AActor* Actor) {
	return Actor->GetClass()->ImplementsInterface(UWeaponAccessInterface::StaticClass());
}

inline AWeapon* GetWeapon(AActor* Actor)
{
	return HasWeaponInterface(Actor) ?
		IWeaponAccessInterface::Execute_GetWeapon(Actor) : nullptr;
}

inline AMeleeWeapon* GetMeleeWeapon(AActor* Actor)
{
	return HasWeaponInterface(Actor) ?
		IWeaponAccessInterface::Execute_GetMeleeWeapon(Actor) : nullptr;
}

inline AFireWeapon* GetFireWeapon(AActor* Actor)
{
	return HasWeaponInterface(Actor) ?
		IWeaponAccessInterface::Execute_GetFireWeapon(Actor) : nullptr;
}

#define CALL_WEAPON_FUNC(Func) if(auto* Weapon = GetWeapon(MeshComp->GetOwner())) Weapon->Func;
#define CALL_MELEE_FUNC(Func) if(auto* Weapon = GetMeleeWeapon(MeshComp->GetOwner())) Weapon->Func;
#define CALL_FIRE_FUNC(Func) if(auto* Weapon = GetFireWeapon(MeshComp->GetOwner())) Weapon->Func;

#pragma endregion

#pragma region AnimNotifies

void UTriggerWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	CALL_WEAPON_FUNC(OnTriggeredDelegate.Broadcast())
}

void UUnJamWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	CALL_WEAPON_FUNC(ReloadLogic())
}

void UReloadWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	CALL_WEAPON_FUNC(SetJammed(false))
}

void UResetHitMemoryMeleeWeapon::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	CALL_MELEE_FUNC(GetMeleeComponent()->ResetActorsToIgnore(bDeepClear))
}

void UMeleeState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	SetActivation(MeshComp, !bInvert); //If invert, deactives
}

void UMeleeState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	SetActivation(MeshComp, bInvert); //If invert, actives
}

void UMeleeState::SetActivation(USkeletalMeshComponent* MeshComp, bool Active)
{
	if (Active) {
		CALL_MELEE_FUNC(Activate(bClearIgnoreActors))
	} else {
		CALL_MELEE_FUNC(Deactivate())
	}

}

#pragma endregion
