// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Bullets/BulletsPoolSystem.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Components/ProjectileBehaviour.h"
#include "Bullets/Bullet.h"
#include "ACSSettings.h"
#include "TimerManager.h"
#include "Bullets/ACSBulletPoolingInterface.h"
#include "ACS_CoreElements.h"

float UBulletsPoolSystem::FillingInterval = 0;
UBulletsPoolSystem* UBulletsPoolSystem::Instance = nullptr;

UBulletsPoolSystem::UBulletsPoolSystem()
{
    //Prevents been collected by Unreal's GC
    AddToRoot();

    Pool, Running, Resting = TArray<AActor*>();
}

void UBulletsPoolSystem::BeginDestroy()
{
    Super::BeginDestroy();
    ClearPools();
}

void UBulletsPoolSystem::ClearPools()
{
    for (auto& i : Pool) {
        i->OnEndPlay.RemoveDynamic(this, &UBulletsPoolSystem::RemoveElement);
        i->Destroy();
    }

    Pool.Empty();
    ClassConcurrency.Empty();
    Running.Empty();
    Resting.Empty();

    for (auto& ClassTimer : ClassesTimers) {
        if (World && World->IsInitialized())    World->GetTimerManager().ClearTimer(ClassTimer.Value);
        else                                    ClassTimer.Value.Invalidate();
    }

    ClassesTimers.Empty();
}

void UBulletsPoolSystem::UpdatePoolRequisites(const TSubclassOf<AActor> Class, const int32 NewRequired, const int32 PreviusRequired)
{
    if (const int32 Diff = NewRequired - PreviusRequired)
    {
        //If pointer exist there's already a requirement
        if (int32* req = PoolRequirements.Find(Class))
        {
            *req += Diff; //Add to the pointer's value the difference

            if(*req <= 0) {
                PoolRequirements.Remove(Class);
                OnRemoveRequirement.Broadcast(Class);
            }

        }
        else if (Diff > 0)
        {
            PoolRequirements.Add(Class, Diff); //Adds the requirements of the class

            //Sets current concurrency to the amount of bullets of the same class that are already handled in pool
            ClassConcurrency.Add(Class, Pool.FilterByPredicate([Class](const AActor* A) {return A && A->GetClass() == Class; }).Num());

            OnAddRequirement.Broadcast(Class);
            OnAddConcurrency.Broadcast(Class);
        }

        InitiatePoolFilling(Class);

    }

}

void UBulletsPoolSystem::PreventDespawningCurrentClass(TSubclassOf<AActor> const Class)
{
    //for each bullet of the class sets the lifespan to 0
    for (AActor* actor : GetPooledBulletsOfClass(Class)) {
        actor->SetLifeSpan(0); //prevent destroy
    }
}

bool UBulletsPoolSystem::RequestElements(   const TArray<FTransform>& Transforms, 
	const TSubclassOf<ABullet>& Class,  const TEnumAsByte<EPoolingMethod::Type> Pooling, TArray<AActor*>& Elements,
    const UBulletInitiationSettings* Settings, AActor* NewOwner
) {

#if WITH_EDITOR

    //Skip in shipping
    if (!Class) {
        //TODO UE_LOG(LogACS, Error, TEXT("Attemted to request bullet without any specified Class"));
        return false;
    }

    if (!Class->ImplementsInterface(UACSBulletPoolingInterface::StaticClass())) {
        //TODO UE_LOG(LogACS, Error, TEXT("The class %s does not implement the interface %s"), Class->GetDisplayNameText().ToString(), UBulletPoolingInterface::StaticClass()->GetDisplayNameText().ToString());
        return false;
    }

#endif

    //Amount of bullets
    const int32 Limit = Transforms.Num();
    Elements.Init(nullptr, Limit);

    //NO POOLLING

    //If not pooling just spawn and forget
    if (Pooling == EPoolingMethod::None) {
        for (int32 i = 0; i < Limit; i++) {
            Elements.AddUnique(World->SpawnActor<AActor>(Class, Transforms[i]));
            ActivateElement(Elements[i], Transforms[i], Settings, NewOwner, false);
        }
        return true;
    }

    int32 count = 0;

    //Should be already sorted by time inactive (trying to use always the same makes the oldest to destroy in dynamic and hybrid saving some memory)
    Elements = Resting.FilterByPredicate([Class, Limit, &count](const AActor* A) {
        //the first part prevents getting more elems than necessary
        return A && A->GetClass() == Class && (count++ < Limit);
    });


    //for each elem in the transform array will iterate
    for (int32 i = 0; i < Limit; ++i) {

        //Uses the inactive bullets when possible
        if (Elements.Num() > i) {
            ActivateElement(Elements[i], Transforms[i], Settings, NewOwner);
        }
        else {
            //If method allows will spawn and activate new bullets
            if (Pooling != EPoolingMethod::Fixed) {

                AActor* Elem = World->SpawnActor<AActor>(Class, Transforms[i]);

                Elements.AddUnique(Elem);

                AddElement(Elem);

                ActivateElement(Elem, Transforms[i], Settings, NewOwner);

            }
            else {
                //if fixed won't spawn more bullets, so end the loop
                UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Tried to spawn %i bullets, but method is fixed and spawned only %i bullets (%i bullets remaining)."), Limit, Elements.Num(), Limit - Elements.Num())
                break;
            }
        }
    }

    //if the num of bullets generated are the same as transform points, will succeed
    return Elements.Num() == Transforms.Num();

}

bool UBulletsPoolSystem::IsPoolFull(TSubclassOf<AActor> const Class) const
{
    //If exists data return requirements less or equal concurrency, otherwise, false
    return (ClassConcurrency.Contains(Class) && PoolRequirements.Contains(Class))
        ? (*PoolRequirements.Find(Class) <= *ClassConcurrency.Find(Class))
        : true;
}

bool UBulletsPoolSystem::IsClassOnLimit(TSubclassOf<AActor> const Class) const
{
    //If exists data return requirements equal concurrency, otherwise, false
    return (ClassConcurrency.Contains(Class) && PoolRequirements.Contains(Class))
        ? (*PoolRequirements.Find(Class) == *ClassConcurrency.Find(Class))
        : false;
}

UBulletsPoolSystem* UBulletsPoolSystem::GetBulletsPoolInstance()
{
    return Instance; //returns the pointer of the static variable
}

const UBulletsPoolSystem* const UBulletsPoolSystem::CreateBulletsPoolInstance(UObject* WorldContextObject, bool Override)
{

    //if not exists or override create a new instance
    if (!Instance || Override) {
        UBulletsPoolSystem::FillingInterval = UACSSettings::Get()->PoolFillInterval;

        if (Instance) {
            Instance->DestroyBulletsPoolInstance();
        }

        Instance = NewObject<UBulletsPoolSystem>();
        Instance->OnAddRequirement.AddDynamic(Instance, &UBulletsPoolSystem::InitiatePoolFilling);
        Instance->OnRemoveRequirement.AddDynamic(Instance, &UBulletsPoolSystem::InitiatePoolFilling);

        //Saves the world from the world context
        Instance->World = WorldContextObject->GetWorld();

    }
    else {
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attemted to create new UBulletsPoolSystem instance while one already exists and is set to do not Override"))
    }

    return Instance;
}

void UBulletsPoolSystem::DestroyBulletsPoolInstance()
{
    RemoveFromRoot();
    ClearPools();
    Instance = nullptr;
}

bool UBulletsPoolSystem::AddElement(AActor* const Elem)
{

    //If is an Actor class and isn't in pool cancel
    if(Elem && Cast<IACSBulletPoolingInterface>(Elem) && !Pool.Contains(Elem)) {

        //Add element to start pooling
        Pool.AddUnique(Elem);

        //Add an instance to the concurrency
        RegisterClass(Elem->GetClass(), true);

        //Select State
        (IsActivated(Elem) ? Running : Resting).AddUnique(Elem);
        //Bind end so can remove from pool automatically
        Elem->OnEndPlay.AddDynamic(this, &UBulletsPoolSystem::RemoveElement);

        return true;

    }

    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to add an element that is invalid, does not contain the ACSBulletPoolingInterface or is already pooled"))
    return false;

}

void UBulletsPoolSystem::RegisterClass(TSubclassOf<AActor> const Class, const bool bAdd)
{
    if (int32* ptr = ClassConcurrency.Find(Class)) {
        int32& amount = *ptr;
        amount += bAdd ? 1 : -1;
        if (amount <= 0) {
            //When the concurrency removes the class and stops the filling
            ClassConcurrency.Remove(Class);
            OnRemoveConcurrency.Broadcast(Class);
            StopPoolFilling(Class);
        }
        else if (int32* minReq = PoolRequirements.Find(Class)) {
            if (amount <= *minReq) {
                if (amount == *minReq) StopPoolFilling(Class);
                PreventDespawningCurrentClass(Class);
            }
        }
    }
    else if (bAdd) { //if not exist only can add, not remove
        ClassConcurrency.Add(Class, 1);
        OnAddConcurrency.Broadcast(Class);
        InitiatePoolFilling(Class);
    }
}

void UBulletsPoolSystem::StopPoolFilling(TSubclassOf<AActor> const Class)
{
    //Finds the timer for the class and clears it
    if (ClassesTimers.Contains(Class))
    {
        FTimerHandle Timer;
        ClassesTimers.RemoveAndCopyValue(Class, Timer);
        World->GetTimerManager().ClearTimer(Timer);
    } else {
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to stop filling a non pool class"))
    }
}

bool UBulletsPoolSystem::RemoveElement(AActor* const Element)
{
    //If handled remove it
    if (Pool.Contains(Element)) {

        //Remove from the pools
        Pool.Remove(Element);
        (Running.Contains(Element) ? Running : Resting).Remove(Element);

        TSubclassOf<AActor> Class = Element->GetClass();

        //Tells that a instance has been removed
        RegisterClass(Class, false);

        //If after removing an instance equals the required pool size, prevent to despawn any other
        if (IsClassOnLimit(Class)) {
            PreventDespawningCurrentClass(Class);
        }

        return true;
    }
    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to remove a not pooled actor"))
    return false;
}

void UBulletsPoolSystem::RemoveElement(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
    if(RemoveElement(Actor))
        Actor->OnEndPlay.RemoveDynamic(this, &UBulletsPoolSystem::RemoveElement);
}

bool UBulletsPoolSystem::SetActivation(AActor* const Element, const bool bActive)
{
    //If the Element is handled, changes position in the array
    if (Pool.Contains(Element)) {

        (bActive ? Resting : Running).Remove(Element);

        //Insert in first position to be accessed earlier and destroy the olders
        (bActive ? Running : Resting).Insert(Element, 0);
        
        //if is active or the pool still require more instances won't auto destroy
        Element->SetLifeSpan(bActive || !IsPoolFull(Element->GetClass()) ? 0 : IACSBulletPoolingInterface::Execute_GetInactiveLimit(Element));

        return true;
    }
    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to activate/deactivate a not pooled actor"))
    return false;
}

bool UBulletsPoolSystem::ActivateElement(AActor* const Element, const FTransform& Transform, const UBulletInitiationSettings* Settings, AActor* NewOwner, bool bIsHandled)
{
    if (Instance && Instance->Pool.Contains(Element)) {

        Instance->Resting.Remove(Element);

        //Insert in first position to be accessed earlier and destroy the olders
        Instance->Running.Insert(Element, 0);

    } 
    else if(bIsHandled) { 
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("The actor %s is told is handled but it is not handled by the PoolSystem"), *Element->GetName())
        return false;
    }

    //If was correctly handled or expected not to be handled, can continue normaly
    Element->SetLifeSpan(0);

    Element->SetOwner(NewOwner);

    IACSBulletPoolingInterface::Execute_ActivateBullet(Element, Transform, Settings);

    return true;
}

bool UBulletsPoolSystem::DeactivateElement(AActor* const Element)
{
    //If the Element is handled, changes position in the array
    if (Instance && Instance->Pool.Contains(Element)) {

        Instance->Running.Remove(Element);

        //Insert in first position to be accessed earlier and destroy the olders
        Instance->Resting.Insert(Element, 0);

        //if is active or the pool still require more instances won't auto destroy
        Element->SetLifeSpan(!Instance->IsPoolFull(Element->GetClass()) ? 0 : IACSBulletPoolingInterface::Execute_GetInactiveLimit(Element));

        return true;
    }
    UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to deactivate a not pooled actor"))
    return false;
}

void UBulletsPoolSystem::InitiatePoolFilling(TSubclassOf<AActor> const Class)
{
    //Creates a timer and a delegated to execute the fill function
    if (World && !ClassesTimers.Contains(Class) && World->IsInitialized()) {
        FTimerHandle TimerHandle;
        ClassesTimers.Add(Class, TimerHandle);
        FTimerDelegate TimerDelegate;
        TimerDelegate.BindUObject(this, &UBulletsPoolSystem::Fill, Class);
        World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, FillingInterval, true);
    }
    else {
        UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to initiate a filling of a class already filling, or world is invalid or not initialized"))
    }
}

void UBulletsPoolSystem::Fill(TSubclassOf<AActor> const Class)
{
    if (int32* required = PoolRequirements.Find(Class)) {
        if (int32* current = ClassConcurrency.Find(Class)) {
            //both should be valid pointers

            //if there are too many reduce the number
            if (*current > *required) {
                int32 count = *current;

                //Gets a bullets until reached required. Gets bullets valid, from the correct class and without a life timer
                TArray<AActor*> ExtraElements = Resting.FilterByPredicate([Class, required, &count](const AActor* A) {
                    return (count-- > *required) && A && A->GetClass() == Class && !A->GetLifeSpan();
                });

                //Sets the bullets to destroy, one each 0.1s
                for (int32 i = 0; i < ExtraElements.Num(); i++)
                {
                    ExtraElements[i]->SetLifeSpan(0.1 * i * IACSBulletPoolingInterface::Execute_GetInactiveLimit(ExtraElements[i]));
                }

                //Stops the filling since everything is already setted up
                StopPoolFilling(Class);
            }

            //if there are not enought bullets spawn one more
            else if (*current < *required) {
                AActor* Actor = World->SpawnActorDeferred<AActor>(Class, FTransform());
                AddElement(Actor);
                Actor->FinishSpawning(FTransform());
                Actor->SetActorHiddenInGame(true);
            }
            else {
                //otherwise the ammount is the same
                StopPoolFilling(Class);
            }
        }
        else StopPoolFilling(Class);
    }
    else StopPoolFilling(Class);
}

bool UBulletsPoolSystem::AddManyElements(TArray<AActor*> const Elements)
{
    //only if all are added
    bool All = true;

    for (AActor* i : Elements) {
        All &= AddElement(i);
    }

    return All;
}

void UBulletsPoolSystem::GetClassesPooled(TArray<TSubclassOf<AActor>>& Classes) const
{
    ClassConcurrency.GenerateKeyArray(Classes);
}

TArray<AActor*> UBulletsPoolSystem::GetActiveBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass) const
{
    //Returns directly the filtered array based in the class
    if (AllowSubclass)  return Running.FilterByPredicate([Class](const AActor* A) {return A && A->IsA(Class); });
    else                return Running.FilterByPredicate([Class](const AActor* A) {return A && A->GetClass() == Class; });
}

TArray<AActor*> UBulletsPoolSystem::GetInactiveBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass) const
{
    //Returns directly the filtered array based in the class
    if (AllowSubclass)  return Resting.FilterByPredicate([Class](const AActor* A) {return A && A->IsA(Class); });
    else                return Resting.FilterByPredicate([Class](const AActor* A) {return A && A->GetClass() == Class; });
}

TArray<AActor*> UBulletsPoolSystem::GetPooledBulletsOfClass(TSubclassOf<AActor> const Class, bool AllowSubclass) const
{
    //Returns directly the filtered array based in the class
    if (AllowSubclass)  return Pool.FilterByPredicate([Class](const AActor* A) {return A && A->IsA(Class); });
    else                return Pool.FilterByPredicate([Class](const AActor* A) {return A && A->GetClass() == Class; });
}

bool UBulletsPoolSystem::IsInPool(AActor* Element)
{
    return Instance && Instance->Pool.Contains(Element);
}

bool UBulletsPoolSystem::IsActivated(AActor* Element)
{
    return Instance && Instance->Running.Contains(Element);
}