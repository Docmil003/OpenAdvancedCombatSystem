// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ACSCoreWeapons.h"
#include "ACS_CoreElements.h"
#include "GameFramework/Character.h"
#include "WeaponAccessInterface.h"
#include "Weapon.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FAmmoTypeChanged, int32, PreviousAmmoInMagazine, TEnumAsByte<EAmmoType>, PreviousAmmo, TEnumAsByte<EAmmoType>, NewAmmo);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSaveRequest, AWeapon*, Weapon, UWeaponInitializationData*, SaveData);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FTrigger);


/** The acs weapons api. */
UCLASS()
class ACS_WEAPONS_API  AWeapon : public AActor, public IWeaponAccessInterface
{
	GENERATED_BODY()

public:
	/** Sets default values for this pawn's properties. */
	AWeapon();

#pragma region Initialization
	
public:

	/** Object with information describing the initialization weapon */
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere, Category=Initialization, meta = (ExposeOnSpawn = "true"))
	TObjectPtr<class UWeaponInitializationData> InitializationWeaponData;

	/** Class of the the weapon data */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Initialization)
	TSubclassOf<class UWeaponInitializationData> InitializationWeaponDataClass;

	/** Indicates the type of weapon. */
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category=Core)
	TEnumAsByte<EWeapon> WeaponType;

	/** Socket to attach at owner mesh. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category=Core)
	FName SocketAttachment;

	/**
	 * Initialization process. Please, feel free to override this function to set the member
	 * variables as needed.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Initialization)
	void Initiate();

	/** Attachs to its owner main mesh. Override if has to attach to a different mesh. */
	UFUNCTION(BlueprintCallable, Category="Utils|Attachment")
	virtual void AttachToOwner();

	/**
	 * Called before calling the save data delegate. Can prepare to modify the data object to match
	 * the weapon status if needed.
	 */
	UFUNCTION(BlueprintNativeEvent, Category=Deinitialization)
	void PreSaveWeaponData();

	/** Calls the delegate of OnSaveRequest */
	UFUNCTION(BlueprintCallable, Category=Deinitialization)
	void SaveData();

protected:

	/** Indicate if the weapons has been initialized. */
	uint8 bIsInitialized;

#pragma region GETTERS
public:

	/**
	 * Indicates if the weapon has been correctly initialized.
	 * @returns	True if initialized, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Initialization)
	bool IsInitialized() const { return !!bIsInitialized; }

#pragma endregion

#pragma endregion

#pragma region Inherance
protected:

	/** Delegate called whenever request to save it's settings. */
	UPROPERTY(BlueprintAssignable, Category=Deinitialization)
	FOnSaveRequest OnSaveDataRequest;

	/**
	 * Executes the 'construction' action.
	 */
	virtual void OnConstruction(const FTransform& Transform) override;

	/** Called when the game starts or when spawned. */
	virtual void BeginPlay() override;

	/**
	 * Called when will be destroyed or game end.
	 * @param 	EndPlayReason	The end play reason.
	 */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

#pragma endregion

#pragma region Modifications
protected:

	/**
	 * Key byte is meant to be used as the slot where the modificator can be changed, while the
	 * value is the byte of the using component.
	 */
	UPROPERTY(EditAnywhere, Category=Modificators)
	TMap<uint8, uint8> Modificators;
	
	/** Enum used as the list of slots that can be modified. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Modificators)
	TObjectPtr<UEnum> SlotsEnum;

	/**
	 * Array of enum used as avaible mods for each slot. Enums should be the same order as the slots
	 * enum.
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Modificators)
	TArray<TObjectPtr<UEnum>> ModsPerSlot;

public:

	/** Forces all modificators to refresh. Will force the size slots to be as the size of mods per slots*/
	UFUNCTION(BlueprintCallable, CallInEditor, Category=Modificators)
	void ApplyModificators();

#pragma region SETTERS
public:

	/**
	 * Updates a modificator in a specific slot. Must be overrided in order to work add features.
	 * @param 	Slot			The slot where the modification is applied.
	 * @param 	Modification	The modification to apply.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category=Modificators)
	void UpdateMod(uint8 Slot, uint8 Modification);

	/**
	 * Updates all modificators to match the input.
	 * @param 	Update	Map containing the new mods and slots.
	 */
	UFUNCTION(BlueprintCallable, Category=Modificators)
	void UpdateMods(const TMap<uint8, uint8>& Update);

#pragma endregion

#pragma region GETTERS
public:

	/** Gets the modificators */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Modificators)
	FORCEINLINE TMap<uint8, uint8> GetModificators() const { return Modificators; }

	/** Gets mods per the modificators enum on each slot */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Modificators)
	FORCEINLINE TArray<UEnum*> GetModsPerSlot() const { return ModsPerSlot; };

	/** Gets the enum of slots that contains modificators */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category=Modificators)
	FORCEINLINE UEnum* GetSlotsEnum() const { return SlotsEnum; };

	/**
	 * Gets a enum of modifiers in the selected slot.
	 * @param 	Index	Index of the slot.
	 * @returns	Null if out of bounds, else the modifier in slot.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Modificators)
	FORCEINLINE UEnum* GetModInSlot(int32 Index) const { return Index < ModsPerSlot.Num() ? ModsPerSlot[Index] : nullptr; };

#pragma endregion

#pragma region Debug

#if WITH_EDITORONLY_DATA

	/**
	 * A helper to see slots and its modificators. This exist ony in editor
	 */
	UPROPERTY(VisibleAnywhere, Category = Modificators)
	TArray<FText> ComponentInfo;

#endif

#if WITH_EDITOR
	/** Populates the component info. This exist ony in editor */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = Modificators)
	void GenerateModificatorsGuide();

#endif

#pragma endregion

#pragma endregion

#pragma region Ammo
public:

	/** Called when the ammo type is going to change. */
	UPROPERTY(BlueprintAssignable, Category = Ammo)
	FAmmoTypeChanged OnAmmoTypeChanged;

protected:

	/** Indicates if weapon requires ammo to trigger */
	UPROPERTY(EditAnywhere, Category = Ammo, BlueprintGetter = IsUsingAmmo, BlueprintSetter = SetUsesAmmo)
	bool bUsesAmmo;

	/** Indicates if has a magazine for the ammo. If not pick the ammo straight from the source (if avaible)*/
	UPROPERTY(EditAnywhere, Category = Ammo, BlueprintGetter = IsUsingMagazine, BlueprintSetter = SetUsesMagazine)
	bool bUsesMagazine;

	/** Ammo type used by the weapon. */
	UPROPERTY(EditAnywhere, Category = Ammo, BlueprintGetter = GetAmmoType, BlueprintSetter = SetAmmoType, meta = (EditCondition = "bUsesAmmo"))
	TEnumAsByte<EAmmoType> AmmoType;

#pragma region SETTERS
public:

	/** Sets uses magazine */
	UFUNCTION(BlueprintSetter, Category = Ammo)
	void SetUsesMagazine(bool bNewUseMagazine);

	/** Sets uses ammo. */
	UFUNCTION(BlueprintSetter, Category = Ammo)
	void SetUsesAmmo(bool bNewUsesAmmo);

	/** Sets ammo type. */
	UFUNCTION(BlueprintCallable, BlueprintSetter, Category = Ammo)
	void SetAmmoType(TEnumAsByte<EAmmoType> NewAmmoType);

#pragma endregion

#pragma region GETTERS
public:

	/** Specifies if the weapon requires ammo. */
	UFUNCTION(BlueprintGetter, Category = Ammo)
	FORCEINLINE bool IsUsingAmmo() const { return bUsesAmmo; };

	/** Specifies if the weapon has a Magazine (Ammo is a must to have magazine) */
	UFUNCTION(BlueprintGetter, Category = Ammo)
	FORCEINLINE bool IsUsingMagazine() const { return bUsesAmmo && bUsesMagazine; };

	/** Indicates the ammo type used by the weapon. */
	UFUNCTION(BlueprintGetter, Category = Ammo)
	FORCEINLINE TEnumAsByte<EAmmoType> GetAmmoType() const { return AmmoType; }

#pragma endregion

#pragma endregion

#pragma region Trigger
public:

	/** Trigger mode used by the weapon. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Trigger)
	TEnumAsByte<ETriggerMode::Type> TriggerMode;

	/**
	 * Fire rate. Amount of executable triggers per second
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Trigger)
	float TriggerRate;

	/** When used in triggermode burst, the amount of bullets spawned in a burst. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Trigger, meta = (EditCondition = "TriggerMode==ETriggerMode::Burst"))
	int32 BurstRate;

	/** Indicates where to call the delegate ontrigger. */
	UPROPERTY(EditAnywhere, Category = "Trigger|Execution")
	TEnumAsByte<EExecutionTiming::Type> TriggerExecution;

	/**
	 * Amount of ammo consumed by a trigger. Usually will be 1, but in case your weapon requires
	 * more than one ammo to trigger, modify this.
	 */
	UPROPERTY(EditAnywhere, Category = "Trigger|Ammo", AdvancedDisplay)
	int32 AmmoLoosePerTrigger;

	/**
	 * Only usefull when AmmoLoosePerTrigger is not 1. Indicates if can trigger when has less ammo
	 * than required (yet above 1). For example, having 1 ammo remaining but
	 * requires(AmmoLoosePerTrigger) 2 per shot.
	 */
	UPROPERTY(EditAnywhere, Category = "Trigger|Ammo", AdvancedDisplay, meta = (EditCondition = "AmmoLoosePerTrigger != 1", EditConditionHides))
	bool bTriggerIfAmmoLessThanRequired;

	/**
	 * Executes all processes to trigger. It may not trigger if conditions are not valid
	 * @param 	Anim	The animation to play.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Trigger)
	void Trigger(const FCharWeaponAnim& Anim);

	/** Call on automatic weapons to stop the trigger at the next execution. */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Trigger)
	void TriggerReleased();

	/**
	 * Executes the logic behind a trigger. Will allways execute unless is Jammed
	 * @param 	Anim	The animation to play.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = Trigger)
	void TriggerLogic(const FCharWeaponAnim& Anim);

	/** Called when triggered successfully. */
	UFUNCTION(BlueprintImplementableEvent, Category = Trigger)
	void OnTriggered();

	/**
	 * Indicates if weapon can be triggered.
	 * @returns	True if we can trigger, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = Trigger)
	bool CanTrigger() const;

	/**
	 * Indicates if ammo is enough to be able to successfully shot.
	 * @returns	True if enough ammo to shot, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Trigger|Ammo")
	FORCEINLINE bool HasEnoughAmmoToShot() const { return GetAmmo() >= (bTriggerIfAmmoLessThanRequired ? 1 : AmmoLoosePerTrigger); }

private:

	/** Cooldown between triggers and also the burst and automatic modes. */
	FTimerHandle Cooldown;

	/** Indicates the index of the burst when using burst or automatic mode. */
	int32 BurstIndex;

	/** Indicates if the trigger is still activated/pulled (Automatic) */
	bool bIsTriggering;

#pragma region GETTERS
public:

	/**
	 * Indicates if is trigger is still pressed(Activated) when automatic.
	 * @returns	True if triggering, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Trigger)
	FORCEINLINE bool IsTriggering() const { return bIsTriggering; };

	/**
	 * Gets the current burst/trigger index.
	 * @returns	The current burst index.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Trigger)
	FORCEINLINE int32 GetCurrentBurstIndex() const { return BurstIndex; }

	/**
	 * Returns the remaining time to end the cooldown of the trigger.
	 * @returns	The remaining cooldown.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Trigger)
	float GetRemainingCooldown() const;

	/**
	 * Indicates if the weapon is not ready for the next trigger, in other worlds, if is cooling
	 * down from previous trigger.
	 * @returns	True if cooling, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Trigger)
	FORCEINLINE bool IsCooling() const { return Cooldown.IsValid(); };

#pragma endregion

#pragma endregion

#pragma region Reload
public:

	/**
	 * If true, round will set max to the max avaible or maximun of round capability. Otherwhise a
	 * loop reload anim will be used (Revolvers for example)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ammo|Magazine", meta = (EditCondition = "bUsesAmmo && bUsesMagazine", EditConditionHides))
	bool bReloadByMagazine;

	/**
	 * Indicates if when reloading, the ammo remaining in the Magazine should be lost (only if not
	 * reloading by iterations)
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Reload)
	bool bLooseExtraAmmoOnReload;

	/**
	 * Indicates when ammo should be exchanged in terms of the animation. If there's no reload
	 * animation it should be set to Begin.
	 */
	UPROPERTY(EditAnywhere, Category = Reload)
	TEnumAsByte<EExecutionTiming::Type> AmmoReloadExecution;

	/**
	 * Amount of ammo added in each step of the reload anim. Requires ReloadByMagazine to be false.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Reload, AdvancedDisplay, meta = (EditCondition = "!bReloadByMagazine && bUsesAmmo && bUsesMagazine", EditConditionHides))
	int32 IterationReloadStepAmount;

	/**
	 * Indicates if the weapon can be currently reloaded.
	 * @returns	True if we can reload, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = Reload)
	bool CanReload() const;

	/**
	 * Reload complete by magazine.
	 * @param 	Anim	The animation to play for the reload.
	 */
	UFUNCTION(BlueprintCallable, Category = Reload)
	virtual void ReloadComplete(const FCharWeaponAnim& Anim);

	/**
	 * Handles the reloading by steps.
	 * @param 	Sequence	The sequence of animations to play.
	 */
	UFUNCTION(BlueprintCallable, Category = Reload)
	virtual void ReloadByIterations(const FCharWeaponAnimArrayForIterations& Sequence);

protected:

	/** If true, when reload by iteration will skip stop at next iteration. */
	bool bCancelReload;

	/** Indicates if reloading. */
	UPROPERTY(BlueprintReadOnly, Category = Reload)
	bool bReloading;

	/**
	 * Starts the reloading by steps and returns the anim duration.
	 * @param 	Anim	The animation to play.
	 * @returns	The duration of the animation.
	 */
	virtual float ReloadIterationStart(const FCharWeaponAnim& Anim);

	/**
	 * Executes the reloading loop.
	 * @param 	Anim   	The animation to play.
	 * @param 	EndAnim	The animation to play when end reloading.
	 */
	virtual void ReloadIterationLoop(const FCharWeaponAnim& Anim, const FCharWeaponAnim& EndAnim);

	/**
	 * Ends the reload by steps.
	 * @param 	Anim	The animation to play.
	 */
	virtual void ReloadIterationEnd(const FCharWeaponAnim& Anim);

	/** Initiate info to begin reloading. */
	virtual void InitiateReload();

	/** Ends the reload and set variables as needed. */
	virtual void EndReload();

#pragma region SETTERS
public:

	/** Cancels the reload. ONLY IF RELOADS BY ITERATIONS. */
	UFUNCTION(BlueprintCallable, Category = Reload)
	FORCEINLINE void CancelReload() { bCancelReload = !bReloadByMagazine; };

	/** Automatizes the reload logic calculating the required ammo. */
	UFUNCTION(BlueprintCallable, Category = Reload)
	virtual void ReloadLogic();

	//Returns the ammo required to fill the magazine. If loose ammo on reload will return the capability
	UFUNCTION(BlueprintCallable, Category = Reload)
	FORCEINLINE int32 GetAmmoRequiredToFill() { return bLooseExtraAmmoOnReload ? MagazineCapability : MagazineCapability - *MagazinePtr; }

	/**
	 * Adds the specified ammo to the Magazine and substract it from the source.
	 * @param 	AddAmmo	The ammount of ammo to add to the megazine. If more than
	 * 					avaible won't be modified
	 */
	UFUNCTION(BlueprintCallable, Category = Reload)
	virtual void ReloadLogicManual(int32 AddAmmo);

#pragma endregion

#pragma region GETTERS
public:

	/**
	 * Query if this object is reloading.
	 * @returns	True if reloading, false if not.
	 */

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Reload)
	FORCEINLINE bool IsReloading() const { return bReloading; }

	/**
	 * Query if this object is reload canceled.
	 * @returns	True if reload canceled, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Reload)
	FORCEINLINE bool IsReloadCanceled() const { return bCancelReload; };

#pragma endregion

#pragma endregion

#pragma region Delegates
public:

	/** Called whenever triggered. */
	UPROPERTY(BlueprintAssignable, Category = Trigger)
	FTrigger OnTriggeredDelegate;

	/** Called when try to shot while jammed. */
	UPROPERTY(BlueprintAssignable, Category = Jamming)
	FTrigger OnJammedDelegate;

	/** The on un jammed delegate. */
	UPROPERTY(BlueprintAssignable, Category = Jamming)
	FTrigger OnUnJammedDelegate;

	/** The on reloaded delegate. */
	UPROPERTY(BlueprintAssignable, Category = Reload)
	FTrigger OnReloadedDelegate;

	/** The on reload failed delegate. */
	UPROPERTY(BlueprintAssignable, Category = Reload)
	FTrigger OnReloadFailedDelegate;

#pragma endregion

#pragma region Jamming
public:

	/**
	 * Indicates if weapon can ever get Jammed when trigger. Keep in mind that this won't unjam the
	 * weapon if is already jammed.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Jamming)
	bool bCanJam;

	/**
	 * Probability of jamming per shot.
	 * @returns	The jam probability.
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = Jamming, meta = (ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1, EditCondition="bCanJam"))
	double JamProbability;

	/**
	 * Indicates when should unjam in terms of the animation. If there's no reload animation it
	 * should be set to Begin.
	 * @returns	The un jam execution timing.
	 */
	UPROPERTY(EditAnywhere, Category = Jamming, meta=(EditCondition = "bCanJam"))
	TEnumAsByte<EExecutionTiming::Type> UnJamExecution;

	/** Sets to get randomly jammed. */
	UFUNCTION(BlueprintCallable, Category = Jamming)
	void RandomJam();

	/**
	 * Unjams the weapon (only  if jammed)
	 * @param 	Anim	The animation to play for unjamming.
	 */
	UFUNCTION(BlueprintCallable, Category = Jamming)
	virtual void UnJam(const FCharWeaponAnim& Anim);

protected:

	/** Indicates if weapon is jammed. */
	bool bJammed = false;

#pragma region SETTERS
public:

	/**
	 * Sets the jamm (only logic so instant, no anim)
	 * @param 	NewJammed	Indicates the new value of Jammed
	 */
	UFUNCTION(BlueprintCallable, Category = Jamming)
	virtual void SetJammed(bool NewJammed);

#pragma endregion

#pragma region GETTERS
public:

	/**
	 * Gets if weapon is jammed.
	 * @returns	True if jammed, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Jamming)
	FORCEINLINE bool IsJammed() const { return bJammed; };
#pragma endregion

#pragma endregion

#pragma region Magazine
protected:

	/** Max ammount of ammo that can be stored per Magazine */
	UPROPERTY(EditAnywhere, Category = "Ammo|Magazine", BlueprintGetter = GetMagazineCapability, BlueprintSetter = SetMagazineCapability, meta = (EditCondition = "bUsesAmmo"))
	int32 MagazineCapability;

	/** Pointer to the variable that holds the ammo for this weapon. */
	FSimplePtr<int32> MagazinePtr;

	/** Pointer to the source of the ammo to get. */
	FSimplePtr<int32> SourceAmmoPtr;

	//Makes sure the magazine is set to its correct value
	void ResolverMagazinePointer();

	template <typename T>
	/** Ensures that the magazine pointer is pointing to the correct variable. */
	FORCEINLINE void EnsurePointer(FSimplePtr<T>& Ptr, const T& NewValue){ if (!Ptr) Ptr.SetPtrDynamic(T(NewValue)); }

#pragma region SETTERS
public: 

	/**
	 * Sets the new capability of the magazine. Manages the previous ammo on its own to prevent ammo
	 * loose.
	 * @param 	NewCapability	The new max capability of the magazine.
	 */
	UFUNCTION(BlueprintSetter, Category = "Ammo|Magazine")
	void SetMagazineCapability(int32 NewCapability);

	/**
	 * Overrides the amount of ammo in the magazine. It does not manage the ammo so won't prevent
	 * loose.
	 * @param 	NewAmmo	The new ammo.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ammo|Magazine")
	void SetAmmoInMagazine(int32 NewAmmo);

	//Sets the souce of ammo from the variable passed by reference
	UFUNCTION(BlueprintCallable, Category = "Ammo|Magazine")
	void SetSourceForAmmo(UPARAM(ref) int32& SourceVariable) { SourceAmmoPtr.SetPtrStatic(SourceVariable); }

	/**
	 * Sets the souce of ammo from the value of a map when the key is the ammo type used.
	 * @param 	Map Will search for the ammo type and use its content as source.
	 * @returns	True if it has that ammo type, false if not.
	 */
	UFUNCTION(BlueprintCallable, Category = "Ammo|Magazine")
	bool SetSourceForAmmoFromMap(UPARAM(ref, DisplayName = "Map Container") TMap<TEnumAsByte<EAmmoType>, int32>& Map);

#pragma endregion

#pragma region GETTERS
public: 

	/**
	 * Gets the max ammount of Ammo that can be in the magazine.
	 * @returns	The magazine capability.
	 */
	UFUNCTION(BlueprintGetter, Category = "Ammo|Magazine")
	FORCEINLINE int32 GetMagazineCapability() const { return MagazineCapability; }

	/**
	 * Gets the ammount of ammo in the magazine. If does not use magazine or the cointainer is
	 * unkown (nullptr) will return 0.
	 * @returns	The ammo in magazine.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ammo|Magazine")
	int32 GetAmmoInMagazine() const { return IsUsingMagazine() && MagazinePtr ? *MagazinePtr : 0; }

	/**
	 * Returns the ammo from the source container (ptr). If does not use ammo returns the 
	 * MagazineCapability (or AmmoLoosePerTrigger if not use magazine) for convinience.
	 * @returns	The ammo from source.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Ammo)
	FORCEINLINE int32 GetAmmoFromSource() const { return bUsesAmmo ? (SourceAmmoPtr ? *SourceAmmoPtr : 0) : (bUsesMagazine ? MagazineCapability : AmmoLoosePerTrigger); }

	/**
	 * Gets the ammo used when trigger. Gets from magazine or source automatically.
	 * @returns	The ammo currently directly accessible.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = Ammo)
	FORCEINLINE int32 GetAmmo() const { return IsUsingMagazine() ? GetAmmoInMagazine() : GetAmmoFromSource(); };

#pragma endregion

#pragma endregion

#pragma region Utils
public:

	/**
	 * Returns the skeletal mesh of itself. Please, if this actor contains more than one SKMesh
	 * override this function to return which acts as main skeletalmesh.
	 * @returns	Null if it fails, else the weapon mesh.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, BlueprintNativeEvent, Category = "Utils|Components", meta = (CompactNodeTitle = "WeaponMesh", HideSelfPin, DefaultToSelf))
	UMeshComponent* GetWeaponMesh() const;

	/**
	 * Returns a reference to its owner which derives from Character class.
	 * @returns	Null if it fails, else the owner character.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils|Casting", meta = (CompactNodeTitle = "OwnerCharacter", HideSelfPin, DefaultToSelf))
	FORCEINLINE ACharacter* GetOwnerCharacter() const { return Cast<ACharacter>(GetOwner()); };

	/**
	 * Returns a reference to the main owner's skeletal mesh, which will be the main mesh of a
	 * Character actor.
	 * @returns	Null if it fails, else the owner mesh.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Utils|Casting", meta = (CompactNodeTitle = "OwnerMesh", HideSelfPin, DefaultToSelf))
	FORCEINLINE USkeletalMeshComponent* GetOwnerMesh() const { return GetOwnerCharacter() ? GetOwnerCharacter()->GetMesh() : nullptr; };

	/**
	 * Plays an animation on the weapon and its owner mesh and returns the time required by the
	 * owner's animation.
	 * @param 	AnimToPlay				   	The animation to play.
	 * @param 	ReturnCharacterAnimDuration	(Optional) True to return character animation 
	 * 										duration if false will use the weapon's.
	 * @returns	Duration of the animation.
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Utils|Animation")
	float PlayAnimStruct(const FCharWeaponAnim& AnimToPlay, const bool ReturnCharacterAnimDuration = true);

	/**
	 * Play animation synchronous.
	 * @param	CharacterTarget		 	Character target to play the animation.
	 * @param	WeaponTarget		 	Weapon target to play the animation.
	 * @param	AnimToPlay			 	The animation to play.
	 * @param	CharacterAnimDuration	Duration of the character animation.
	 * @param	WeaponAnimDuration   	Duration of the weapon animation.
	 */
	UFUNCTION(BlueprintCallable, Category = "Utils|Animation")
	static void PlayAnimSynchronous(ACharacter* CharacterTarget, AWeapon* WeaponTarget, const FCharWeaponAnim& AnimToPlay, float& CharacterAnimDuration, float& WeaponAnimDuration);

#pragma endregion

#pragma region TriggerBind
protected:
	/** Clears the cooldown so it can be trigger again. */
	UFUNCTION()
	virtual void EndTriggerCooldown();

	/**
	 * Handles the trigger when having a burst.
	 * @param 	Anim	The animation to play on the trigger.
	 */
	UFUNCTION()
	virtual void TriggerBurst(const FCharWeaponAnim& Anim);

	/**
	 * Handles the auto trigger in automatic weapons.
	 * @param 	Anim	The animation to play on the trigger.
	 */
	UFUNCTION()
	virtual void AutoTrigger(const FCharWeaponAnim& Anim);

#pragma endregion

	/**
	 * Gets weapon implementation.
	 * @returns	Null if it fails, else the weapon implementation.
	 */
	virtual AWeapon* GetWeapon_Implementation() override { return this; };

};
