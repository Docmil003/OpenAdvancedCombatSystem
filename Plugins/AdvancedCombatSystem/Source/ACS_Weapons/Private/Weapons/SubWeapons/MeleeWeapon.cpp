// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Weapons/SubWeapons/MeleeWeapon.h"

#pragma region Main

void AMeleeWeapon::Activate(bool bReset)
{
#if !NO_LOGGING

    if (!GetMeleeComponent())
    {
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a melee component to activate"))
        return;
    }

#endif // DEBUG

    if (CanActivate()) GetMeleeComponent()->Activate(bReset);
}

void AMeleeWeapon::Deactivate()
{

#if !NO_LOGGING

    if (!GetMeleeComponent())
    {
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Can not get a melee component to deactivate"))
        return;
    }

#endif // DEBUG

    if(CanDeactivate()) GetMeleeComponent()->Deactivate();
}

#pragma endregion

#pragma region GET ONE

UMeleeComponent* AMeleeWeapon::GetMeleeComponentByTag(const FName& Tag) const
{
    TArray<UActorComponent*> Comp = GetComponentsByTag(UMeleeComponent::StaticClass(), Tag);
    return Comp.IsEmpty() ? nullptr : Cast<UMeleeComponent>(Comp[0]);
}

UMeleeComponent* AMeleeWeapon::GetActiveMeleeComponent() const
{
    TArray<UMeleeComponent*> Components;
    GetAllMeleeComponents(Components);

    for (UMeleeComponent* i : Components) {
        if (i->IsActive()) return i;
    }

    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Any melee component is activated"))

    return nullptr;
}

UMeleeComponent* AMeleeWeapon::GetInactiveMeleeComponent() const
{
    TArray<UMeleeComponent*> Components;
    GetAllMeleeComponents(Components);

    for (UMeleeComponent* i : Components) {
        if (!i->IsActive()) return i;
    }

    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Any melee component is deactivated"))

    return nullptr;
}

#pragma endregion

#pragma region GET MULTIPLE

void AMeleeWeapon::GetAllActiveMeleeComponents(TArray<UMeleeComponent*>& Components) const
{
    GetAllMeleeComponents(Components);
    Components = Components.FilterByPredicate([](const UMeleeComponent* A) {
        return A->IsActive();
    });
}

void AMeleeWeapon::GetAllInactiveMeleeComponents(TArray<UMeleeComponent*>& Components) const
{
    GetAllMeleeComponents(Components);
    Components = Components.FilterByPredicate([](const UMeleeComponent* A) {
        return !A->IsActive();
    });
}

void AMeleeWeapon::GetAllMeleeComponents(TArray<UMeleeComponent*>& Components) const
{
    GetComponents<UMeleeComponent*>(Components);
#if !NO_LOGGING
    if (Components.IsEmpty()) UE_LOG(AdvancedCombatSystem_Weapons_Log, Error, TEXT("Actor %s does not have any melee component"), *GetName())
#endif
}

#pragma endregion