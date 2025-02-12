// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ACSCoreHealth.h"
#include "Components/ActorComponent.h"

#include "HealthComponent.generated.h"

UENUM(BlueprintType)
namespace EHealthCombinationMethod {
	/** Values that represent types */
	enum Type {
		Combine,
		Average,
		Max,
		Min,
		Custom1,
		Custom2,
		Custom3,
	};
}

#pragma region Forward Declaration

struct FHealth;
struct FDefenseProperties;
struct FDamageInfo;
struct FHealInfo;
struct FActorDamageInfo;

#pragma endregion

#pragma region Delegates Declaration

//C++

DECLARE_MULTICAST_DELEGATE_TwoParams(FDamageReceived, const AActor*, const FDamageInfo&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FHealthReceived, const AActor*, const FHealInfo&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FComponentHealed, const AActor*, const FHealth&, const FHealInfo&);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FComponentDamaged, const AActor*, const FHealth&, const FDamageInfo&);

//BP

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDamageReceivedBP, const AActor*, Actor, const FDamageInfo&, Damage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthReceivedBP, const AActor*, Actor, const FHealInfo&, Heal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComponentHealedBP, const AActor*, Actor, const FHealth&, HealthComp, const FHealInfo&, Heal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComponentDamagedBP, const AActor*, Actor, const FHealth&, HealthComp, const FDamageInfo&, Damage);

#pragma endregion

#pragma region TYPEDEF

/** Copy group */
typedef TPair<FHealth, FDefenseProperties> FHealthPair;
typedef TPair<FHealth&, FDefenseProperties&> FHealthPairRef;
typedef TMap<FHealth, FDefenseProperties> FHealthMap;

/** Ptr group */
typedef TPair<FHealth*, FDefenseProperties*> FHealthPtrPair;
typedef TMap<FHealth*, FDefenseProperties*> FHealthPtrMap;

#pragma endregion

/**
* A class to simulate an advanced health and armor system
*/
UCLASS( ClassGroup=(AdvancedCombatSystem), Blueprintable, meta=(BlueprintSpawnableComponent), meta=(ToolTip="Simulates the health of the actor by managing damage based on damage type and bone impact, as well as incorporates the main health and an array of armor that can be equipped", ShortToolTip="Component to simulate actor's Health"))
class ACS_HEALTH_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** Sets default values for this component's properties */
	UHealthComponent();

#pragma region Hinerance
protected:
	/** Called when the game starts */
	virtual void BeginPlay() override;

#pragma endregion

#pragma region Health
protected:
	/** Main health structure Pair*/
	FHealthPairRef Health = FHealthPairRef(InitialHealth, InitialDefense);

private:

	//Main health structure
	UPROPERTY(EditAnywhere, Category = "Health", BlueprintGetter = GetHealthComponent, BlueprintSetter = SetHealthComponent, meta = (DisplayName = "Health", ShowOnlyInnerProperties))
	FHealth InitialHealth;

	//Main health's Defense values
	UPROPERTY(EditAnywhere, Category = "Health", BlueprintGetter = GetHealthDefenseComponent, BlueprintSetter = SetHealthDefenseComponent, meta = (DisplayName = "Defense", ShowOnlyInnerProperties))
	FDefenseProperties InitialDefense;

#pragma region SETTERS & GETTERS
public:

	/******************************************************
	*******************SETTERS
	*******************************************************/

	/**
	 * Sets the new health. Use with care when health is binded
	 * @param 	NewHealth	The new health.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthComponent(const FHealth& NewHealth);

	/**
	* Sets the new defense form main health
	* @param 	NewDefense	The new damage defense.
	*/
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealthDefenseComponent(const FDefenseProperties& NewDefense);

	/**
	 * Sets a the health of the main health
	 * @param 	NewHealth	The new health.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetHealth(double NewHealth);

	/**
	 * Sets new main's maximum health
	 * @param 	NewMaxHealth	The new maximum health.
	 */
	UFUNCTION(BlueprintCallable, Category = "Health")
	void SetMaxHealth(double NewMaxHealth);

	/**
	 * Set the new health and defense
	 * @param 	NewHealthPair	The new health defense pair.
	 */
	void SetHealthPair(const FHealthPair& NewHealthPair);


	/******************************************************
	*******************GETTERS
	*******************************************************/

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	FORCEINLINE double GetHealth() const { return *Health.Key.Health; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	FORCEINLINE double GetMaxHealth() const { return *Health.Key.MaxHealth; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	const FORCEINLINE FHealth& GetHealthComponent() const { return Health.Key; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	FORCEINLINE FDefenseProperties& GetHealthDefenseComponent() { return Health.Value; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	bool IsAlive() { return *Health.Key.Health > 0; };

#pragma endregion

#pragma endregion

#pragma region Armor
protected:

	//Array of applied armors
	UPROPERTY(EditAnywhere, Category = "Armor", BlueprintGetter = GetArmor, BlueprintSetter = SetArmor, meta = (DisplayName = "Armor", ShowOnlyInnerProperties))
	TMap<FHealth, FDefenseProperties> Armor;

#pragma region SETTERS & GETTERS
public:

	/******************************************************
	*******************SETTERS
	*******************************************************/

	/**
	 * Set the array of Armor
	 * @param 	NewArmor	The new armor have.
	 */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void SetArmor(const TMap<FHealth, FDefenseProperties>& NewArmor);

	/**
	 * Sets armor's health for the selected index
	 * @param 	NewArmor	The new health of the armor.
	 * @param 	Index   	(Optional) Index of the armor to modify.
	 */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void SetArmorHealth(double NewArmor, const int32 Index = 0);

	/**
	 * Sets maximum armor's health for the seleced index
	 * @param 	NewMaxArmor	The new maximum health for the armor.
	 * @param 	Index	   	(Optional) Index of the armor to modify.
	 */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void SetMaxArmorHealth(double NewMaxArmor, const int32 Index = 0);

	/**
	 * Sets armor's health component
	 * @param 	NewHealth	The new health for the armor component.
	 * @param 	Index	 	(Optional) Index of the armor to modify.
	 */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void SetArmorComponent(const FHealth& NewHealth, const int32 Index = 0);

	/**
	 * Sets armors defense component
	 * @param 	Newdefense	The new defense for the component.
	 * @param 	Index		(Optional) Index of the armor to modify.
	 */
	UFUNCTION(BlueprintCallable, Category = "Armor")
	void SetArmorsDefenseComponent(const FDefenseProperties& Newdefense, const int32 Index = 0);

	/**
	 * Sets armor pair
	 * @param 	NewArmorPair	The new armor pair.
	 * @param 	Index			(Optional) Index of the armor to modify.
	 */
	void SetArmorPair(const FHealthPair& NewArmorPair, const int32 Index = 0);

	/******************************************************
	*******************GETTERS
	*******************************************************/

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armor")
	const FORCEINLINE TMap<FHealth, FDefenseProperties>& GetArmor() const { return Armor; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armor")
	const FORCEINLINE FHealth& GetArmorComponent(const int32 Index = 0) const { return GetArmorPair(Index).Key; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armor")
	const FORCEINLINE FDefenseProperties& GetArmorsDefenseComponent(const int32 Index = 0) const { return GetArmorPair(Index).Value; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armor")
	FORCEINLINE double GetArmorHealth(const int32 Index = 0) const { return *GetArmorComponent(Index).Health; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Armor")
	FORCEINLINE double GetArmorMaxHealth(const int32 Index = 0) const { return *GetArmorComponent(Index).MaxHealth; }

	const FHealthPair& GetArmorPair(const int32 Index = 0) const;

	void GetArmorPtr(FHealthPtrMap& Map);

#pragma endregion

#pragma endregion

#pragma region DELEGATES

#pragma region C++
private:
	//DAMAGE

	/** Called before the damage takes place */
	FDamageReceived OnDamagedDelegate;

	/** Called when the damage affects a health component */
	FComponentDamaged OnDamagedHealthDelegate;

	/** Called when the damage affects a health component */
	FComponentDamaged OnDamagedArmorDelegate;

	/** Called once the health arrived to 0 */
	FComponentDamaged OnKilledDelegate;

	/** Called when an armor breaks */
	FComponentDamaged OnArmorDestroyedDelegate;


	//HEAL

	/** Called befor the health applies */
	FHealthReceived OnHealedDelegate;

	/** Called when healed while dead */
	FComponentHealed OnRevivedDelegate;

	/** Called when an armor is restored */
	FComponentHealed OnRestoredArmorDelegate;

#pragma endregion

#pragma region BLUEPRINTS
public:

	//DAMAGE

	/** Called before damage takes place */
	UPROPERTY(BlueprintAssignable, Category = "Damage", meta = (DisplayName = "OnDamaged"))
	FDamageReceivedBP OnDamaged_BP;

	/** Called when the damage affects a health component */
	UPROPERTY(BlueprintAssignable, Category = "Damage", meta = (DisplayName = "OnDamagedHealthDelegate"))
	FComponentDamagedBP OnDamagedHealthDelegate_BP;

	/** Trigger when the main life goes below zero */
	UPROPERTY(BlueprintAssignable, Category = "Damage", meta = (DisplayName = "OnKilled"))
	FComponentDamagedBP OnKilled_BP;

	/** Called when the damage affects a health component */
	UPROPERTY(BlueprintAssignable, Category = "Damage", meta = (DisplayName = "OnDamagedArmorDelegate"))
	FComponentDamagedBP OnDamagedArmorDelegate_BP;

	/** Called when an armor health is reduced to 0 or below */
	UPROPERTY(BlueprintAssignable, Category = "Damage", meta = (DisplayName = "OnArmorDestroyed"))
	FComponentDamagedBP OnArmorDestroyed_BP;

	//HEAL

	/** Called when health or armor is recovered */
	UPROPERTY(BlueprintAssignable, Category = "Heal", meta = (DisplayName = "OnHealed"))
	FHealthReceivedBP OnHealed_BP;

	/** Called whenever healts from dead */
	UPROPERTY(BlueprintAssignable, Category = "Heal", meta = (DisplayName = "OnRevived"))
	FComponentHealedBP OnRevived_BP;

	/** Called whenever a broken Armor is restored */
	UPROPERTY(BlueprintAssignable, Category = "Heal", meta = (DisplayName = "OnArmorRestored"))
	FComponentHealedBP OnArmorRestored_BP;

#pragma endregion

#pragma endregion

#pragma region DELEGATES FUNCTIONS

#pragma region C++
private:

	/**
	 * Called when receives damage.
	 * @param 	Actor 	The actor that took the damage.
	 * @param 	Damage	The damage to apply to the actor.
	 */
	void OnDamaged(const AActor* Actor, const FDamageInfo& Damage);

	/**
	 * Called when the main health has suffer damage
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The health component that is damaged.
	 * @param 	Damage	  	The damage info received.
	 */
	void OnDamagedHealth(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Called when the main health has suffer damage
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	void OnDamagedArmor(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Called when Main Health Component is reduced bellow 0
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	void OnKilled(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Executes the 'armor destroyed' action
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	void OnArmorDestroyed(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * HEAL
	 * @param 	Actor	The actor.
	 * @param 	Heal 	The heal.
	 */
	void OnHealed(const AActor* Actor, const FHealInfo& Heal);

	/**
	 * Executes the 'health heal' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	void OnHealthHeal(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'armor heal' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	void OnArmorHeal(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'revived' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	void OnRevived(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'restored armor' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	void OnRestoredArmor(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

#pragma endregion

#pragma region BLUEPRINTS
public:

	/**
	 * Called when receives damage.
	 * @param 	Actor 	The actor that took the damage.
	 * @param 	Damage	The damage to apply to the actor.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage", meta = (DisplayName = "OnDamaged"))
	void OnDamagedBP(const AActor* Actor, const FDamageInfo& Damage);

	/**
	 * Called when the main health has suffer damage
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The health component that is damaged.
	 * @param 	Damage	  	The damage info received.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage", meta = (DisplayName = "OnDamagedHealth"))
	void OnDamagedHealthBP(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Called when the main health has suffer damage
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage", meta = (DisplayName = "OnDamagedArmor"))
	void OnDamagedArmorBP(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Called when Main Health Component is reduced bellow 0
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage", meta = (DisplayName = "OnKilled"))
	void OnKilledBP(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * Executes the 'armor destroyed' action
	 * @param 	Actor	  	The actor that took the damage.
	 * @param 	HealthComp	The component that received the damage.
	 * @param 	Damage	  	The damage received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Damage", meta = (DisplayName = "OnArmorDestroyed"))
	void OnArmorDestroyedBP(const AActor* Actor, const FHealth& HealthComp, const FDamageInfo& Damage);

	/**
	 * HEAL
	 * @param 	Actor	The actor.
	 * @param 	Heal 	The heal.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Heal", meta = (DisplayName = "OnHealed"))
	void OnHealedBP(const AActor* Actor, const FHealInfo& Heal);

	/**
	 * Executes the 'health heal' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Heal", meta = (DisplayName = "OnHealthHeal"))
	void OnHealthHealBP(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'armor heal' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Heal", meta = (DisplayName = "OnArmorHealBP"))
	void OnArmorHealBP(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'revived' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Heal", meta = (DisplayName = "OnRevived"))
	void OnRevivedBP(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);

	/**
	 * Executes the 'restored armor' action
	 * @param 	Actor	  	The actor that took the heal.
	 * @param 	HealthComp	The component that received the heal.
	 * @param 	Heal	  	The heal received by the component.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Heal", meta = (DisplayName = "OnRestoredArmor"))
	void OnRestoredArmorBP(const AActor* Actor, const FHealth& HealthComp, const FHealInfo& Heal);
	
#pragma endregion

#pragma endregion

#pragma region Damage
public:

	/** Method of how damage should be absorbed */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage|Calculation")
	EArmordefenseMethod ArmordefenseMethod;

	/** Indicates if when received damage, can damage multiple armors before damaging the main or not */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage|Calculation")
	bool bOnlyOneArmorDamageAtTime;

	/**   
	 * Indicates if when choosing armor to damage, should add main health to the list. 
	 * Note that it will make possible to the main health to be damaged twice by the same damage 
	 */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage|Calculation")
	bool bIncludeMainHealthOnDamage;

	/**
	 * Damage component the component with the feed data
	 * @param	HealthComp	   	The health component that will receive the harm.
	 * @param 	Absorb		   	The defense properties to calaculate Defense and Scale.
	 * @param	DamageInfo	   	Information describing the damage to deal with.
	 * @param	RemainingDamage	The remaining damage that couldn't be absorbed.
	 * @param 	DamageScale	   	(Optional) The base damage scale.
	 * @returns	The ammount of health that remains.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage|Calculation")
	virtual double DamageComp(FHealth& HealthComp, const FDefenseProperties& Absorb, const FDamageInfo& DamageInfo, UPARAM(ref) double& RemainingDamage, double DamageScale = 1);


protected:

	/**
	 * Damages this component's components
	 * @param 	Actor 	The actor that is receiving the damage.
	 * @param 	Damage	The damage taken.
	 */
	virtual void Damage(const AActor* Actor, const FDamageInfo& Damage);

	/**
	 * Process the damage to the corresponding component
	 * @param 	DamageInfo	Info of the damage that will be used to harm.
	 * @returns	Ammount of damage Gathered in all harms.
	 */
	double ProcessDamage(const FDamageInfo& DamageInfo);
#pragma region STATIC FUNCTIONS
public:

	/**
	 * Gets a scale of how much of the damage should try to absorb
	 * @param 	Absorb	Defense properties.
	 * @param 	Damage	Damage received properties.
	 * @returns	Scale of the damage based in the data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage|Calculation")
	static double CalculateAbsorbption(const FDefenseProperties& Defense, const FDamageInfo& Damage);

	/**
	 * Gets a scale of how much the damage should be reduced
	 * @param 	Absorb	Defense properties.
	 * @param 	Damage	Damage received properties.
	 * @param 	Scale 	(Optional) Initial scale of the damage.
	 * @returns	Scale of the damage based in the data.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage|Calculation")
	static double GetDamageScale(const FDefenseProperties& Absorb, const FDamageInfo& Damage, double Scale = 1);

	/**
	* Damages the owner of the specified component.
	* @param 	Info	ActorDamageInfo to send to the HealthComponent
	* @returns	true if the actor contains a HealthComponent.
	*/
	UFUNCTION(BlueprintCallable, Category = "Damage")
	static bool SendDamageToActorStruct(const FActorDamageInfo& Info);

	/**
	 * Sends damage to an actor
	 * @param 	Target	Target for the damage.
	 * @param 	Damage	The damage to apply.
	 * @returns	True if the actor contains a HealthComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	static bool SendDamageToActor(const AActor* Target, const FDamageInfo& Damage);

	/**
	 * Sends to a list of actors the damage they receive
	 * @param 	List		 	The list of actors and damage received.
	 * @param 	bSkipNoDamage	(Optional) True to skip send when no damage.
	 * @returns	True if at least one actor contains a HealthComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage", meta = (AdvancedDisplay = "bSkipNoDamage"))
	static bool SendDamageToActorsStruct(const TArray<FActorDamageInfo>& List, const bool bSkipNoDamage = true);

	/**
	 * Sends the same damage to many actors
	 * @param 	Target		 	List of actors to send damage to.
	 * @param 	Damage		 	The damage that all actors will receive.
	 * @param 	bSkipNoDamage	(Optional) True to skip send when no damage.
	 * @returns	True if at least one actor contains a HealthComponent
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage", meta = (AdvancedDisplay = "bSkipNoDamage"))
	static bool SendDamageToActors(const TArray<AActor*>& Target, const FDamageInfo& Damage, const bool bSkipNoDamage = true);

	/**
	 * Damages the owner of the specified component.
	 * @param	Comp  	Component of the Actor to harm.
	 * @param 	Damage	DamageInfo to send to the HealthComponent.
	 * @returns	true if the components's owner contains a HealthComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Damage")
	static FORCEINLINE bool SendDamageToOwner(UActorComponent* Comp, const FDamageInfo& Damage) { return Comp ? SendDamageToActor(Comp->GetOwner(), Damage) : false; };

#pragma endregion

#pragma endregion

#pragma region Heal
protected:

	/**
	 * Damages this component's components
	 * @param 	Actor	The actor that receive the heal.
	 * @param 	Heal 	The heal that will be taken.
	 */
	virtual void Heal(const AActor* Actor, const FHealInfo& Heal);

	/**
	 * Process the healing to the corresponding component
	 * @param 	HealthInfo	Information describing the heal to apply. Will update to reflect the
	 * 						raw value or percentage that was applied
	 */
	virtual void ProcessHeal(FHealInfo& HealthInfo);

	/**
	 * Builds and apply health
	 * @param HealthComp	The health component to heal.
	 * @param Info	  		The information to heal the component with.
	 */
	virtual void BuildAndApplyHealth(FHealth& HealthComp, FHealInfo& Info);

	/**
	 * Stars a decrease of the health until is under max
	 * @param Target	Component that will be reduced its health until reached a in range value.
	 */
	void BeginDecreaseExceededHealth(FHealth& Target);

private:

	/**
	 * Handles the over healed described by Element
	 * @param Element	The element that will have the health normalized.
	 */
	virtual void HandleOverHealed(FHealth& Element);

#pragma region STATIC FUNCTIONS
public:

	/**
	* Sends healing information to the owner of the specified component.
	* @param 	Target	Actor to heal.
	* @param 	Heal  	HealthInfo to send to the HealthComponent.
	* @returns	true if the actor contains a HealthComponent.
	*/
	UFUNCTION(BlueprintCallable, Category = "Heal")
	static bool SendHealthToActor(const AActor* Target, const FHealInfo& Heal);

	/**
	 * Sends healing information to the owner of the specified component.
	 * @param 	Target	Actors to heal.
	 * @param 	Heal  	HealthInfo to send to the all the actors.
	 * @returns	true if at least one actor contains a HealthComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Heal")
	static bool SendHealthToActors(const TArray<AActor*>& Target, const FHealInfo& Heal);

	/**
	 * Sends healing information to the owner of the specified component.
	 * @param 	Comp	Component of the actor to heal.
	 * @param 	Heal  	HealthInfo to send to the all the actors.
	 * @returns	true if at least one actor contains a HealthComponent.
	 */
	UFUNCTION(BlueprintCallable, Category = "Heal")
	static FORCEINLINE bool SendHealthToOwner(UActorComponent* Comp, const FHealInfo& Heal) { return Comp ? SendHealthToActor(Comp->GetOwner(), Heal) : false; }

#pragma endregion

#pragma endregion

#pragma region BP ONLY FUNCTIONS
protected:

	/**
	 * Searches for the first health by tag
	 * @param 	TargetTag 	Target tag to search for.
	 * @param	HealthComp	The health component found.
	 * @returns	True if found.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data", meta = (DisplayName = "FindHealthByTag"))
	bool FindHealthByTag_BP(const FName& TargetTag, FHealth& HealthComp);

	/**
	 * Sorts the map components by the greatest Defense to a damage.
	 * @param	Array 	Array of Health-Defense sorted be defense.
	 * @param	Damage	Info of the damage on which Defense values will be calculated.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data", meta = (DisplayName = "SortDamageByDefense"))
	void SortByDefense_BP(TMap<FHealth, FDefenseProperties>& Map, const FDamageInfo& Damage, bool bIncludeMain = false) const;

	/**
	 * Filters the Array components to leave only which are alive.
	 * @param	Array	Array of Health filtered.
	 * @param	bIncludeMain	Indicates if should look also for the Main health or not
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data", meta = (DisplayName = "FilterAliveComponents"))
	void FilterAlive_BP(TArray<FHealth>& Array, bool bIncludeMain = false) const;

	/**
	 * Checks if at least one Health component is alive
	 * @param	bIncludeMain	Indicates if should look also for the Main health or not
	 * @returns	True if any still alive, false if not.
	 */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data", meta = (DisplayName = "IsAnyAlive"))
	bool IsAnyAlive_BP(bool bIncludeMain = false) const;

#pragma endregion

#pragma region Miscellaneous
protected:

	/**
	 * Divide damage on to apply a proportional, unbaked damage
	 * @param	Components	   	The components between the damage will be shared.
	 * @param 	DamageInfo	   	Information describing the damage.
	 * @param	RemainingDamage	The remaining damage that could not be completely taken.
	 * @param	DamageGathered 	The final damage gathered among all the components.
	 */
	void DivideDamageOnElements(const FHealthPtrMap& Components, const FDamageInfo& DamageInfo, double& RemainingDamage, double& DamageGathered);

	/**
	 * Damage the first component on the list of components
	 * @param	Components	   	The list of components.
	 * @param 	DamageInfo	   	Information describing the damage.
	 * @param	RemainingDamage	The remaining damage that could not be taken.
	 * @param	DamageGathered 	The damage gathered.
	 * @returns	The first element of the components after damaging it
	 */
	FHealth* DamageFirst(const FHealthPtrMap& Components, const FDamageInfo& DamageInfo, double& RemainingDamage, double& DamageGathered);

	/**
	 * Handles the damage to the defense Components
	 * @param	Components			The components to handle.
	 * @param 	DamageInfo			Information describing the damage.
	 * @param	UnabsorbedDamage	The unabsorbed damage.
	 * @returns	The ammount of damage that has received in the handle.
	 */
	virtual double HandleDamageDefense(FHealthPtrMap& Components, const FDamageInfo& DamageInfo, double& UnabsorbedDamage);

#pragma endregion

#pragma region Health Sharing
public:

	/**
	 * Sets the components health to be shared with this component (note that defense WILL NOT be
	 * copied)
	 * @param 	TagName			 	Name of the tag of the component to share.
	 * @param 	ShareWith		 	List of actors to share with.
	 * @param 	CombinationMethod	The combination method for health and max health.
	 * @returns	True if one or more actors have been successfully binded.
	 */
	UFUNCTION(BlueprintCallable, Category = "Share")
	virtual bool ShareHealth(const FName& TagName, const TArray<UHealthComponent*>& ShareWith, TEnumAsByte<enum EHealthCombinationMethod::Type> CombinationMethod);

	/**
	 * Sets the components health to be shared with this component (note that defense WILL NOT be
	 * copied)
	 * @param 	ShareComp			Component to share.
	 * @param 	ShareWith		 	List of actors to share with.
	 * @param 	CombinationMethod	The combination method for health and max health.
	 * @returns	True if one or more actors have been successfully binded.
	 */
	FORCEINLINE bool ShareHealth(const FHealth& ShareComp, const TArray<UHealthComponent*>& ShareWith, TEnumAsByte<enum EHealthCombinationMethod::Type> CombinationMethod) { return ShareHealth(ShareComp.Tag, ShareWith, CombinationMethod); };

	/**
	 * Sets the components health to be shared with this component (note that defense WILL NOT be
	 * copied)
	 * @param 	TagName			 	Name of the tag of the component to share.
	 * @param 	ShareWith		 	List of actors to share with.
	 * @param 	CombinationMethod	The combination method for health and max health.
	 * @returns	True if has been successfully binded.
	 */
	FORCEINLINE bool ShareHealth(const FName& TagName, UHealthComponent* ShareWith, TEnumAsByte<enum EHealthCombinationMethod::Type> CombinationMethod) { return ShareHealth(TagName, TArray<UHealthComponent*>({ ShareWith }), CombinationMethod); };

	/**
	 * Sets the components health to be shared with this component (note that defense WILL NOT be
	 * copied)
	 * @param 	ShareComp			Component to share.
	 * @param 	ShareWith		 	List of actors to share with.
	 * @param 	CombinationMethod	The combination method for health and max health.
	 * @returns	True if has been successfully binded.
	 */
	FORCEINLINE bool ShareHealth(const FHealth& ShareComp, UHealthComponent* ShareWith, TEnumAsByte<enum EHealthCombinationMethod::Type> CombinationMethod) { return ShareHealth(ShareComp.Tag, TArray<UHealthComponent*>({ ShareWith }), CombinationMethod); };

	/**
	 * Combination method calculator
	 * @param 	Healths			 	The array of health (x) and max health (y) of the elements.
	 * @param 	CombinationMethod	The combination method.
	 * @returns	A FVector2D representing the result of the operations been X as health and Y as max health.
	 */
	virtual FVector2D CombinationMethodCalculator(const TArray<FVector2D>& Healths, const EHealthCombinationMethod::Type CombinationMethod);

private:

	/**
	 * Sets up the bindings for share health and events
	 * @param	CompToBind	The component to bind.
	 * @param 	IsHealth  	True if is main health, false if is armor.
	 * @param 	BindToComp	(Optional) If is true, this will also be binded to the CompToBind.
	 */
	virtual void SetUpBindings(UHealthComponent* CompToBind, bool IsHealth, bool BindToComp = false);

	/**
	 * Sets up the bindings for the component to itself binding health and armor
	 * @param	CompToBind	The component to bind.
	 */
	void SetUpBindings(UHealthComponent* CompToBind);

	/** Clears the bindings for the specified component */
	virtual void ClearBindings(UHealthComponent* CompToUnbind);

	/** Clears the bindings for the specified components */
	virtual FORCEINLINE void ClearBindings(const TArray<UHealthComponent*>& CompsToUnbind);

#pragma endregion

#pragma region Filter And Sort
protected:

	/**
	 * Sorts the map components by the greatest Defense to a damage.
	 * @param	Map 	Map of Health-Defense (ptr) to use to sort.
	 * @param	Damage	Info of the damage on which Defense values will be calculated.
	 */
	static void SortByDefense(FHealthPtrMap& Map, const FDamageInfo& Damage);

	/**
	 * Sorts the map components by the greatest Defense to a damage.
	 * @param	Array 	Map of Health-Defense to use to sort.
	 * @param	Damage	Info of the damage on which Defense values will be calculated.
	 */
	static void SortByDefense(FHealthMap& Map, const FDamageInfo& Damage);

	/**
	 * Filters the Array components to leave only which are alive.
	 * @param	Array	Array of Health to filter.
	 */
	static void FilterAlive(TArray<FHealth*>& Array);

	/**
	 * Filters the map components to leave only which are alive.
	 * @param	Map	The map to filter.
	 */
	static void FilterAlive(FHealthPtrMap& Map);

	/**
	 * Filters the Array components to leave only which are alive.
	 * @param	Array	Array of Health to filter.
	 */
	static void FilterAlive(TArray<FHealth>& Array);

	/**
	 * Checks if at least one Health component is alive
	 * @param	Array	To check for alive components
	 * @returns	True if any still alive, false if not.
	 */
	static bool IsAnyAlive(const TArray<FHealth*> Array);

	/**
	 * Checks if at least one Health component is alive
	 * @param	Map	To check for alive components
	 * @returns	True if any still alive, false if not.
	 */
	static bool IsAnyAlive(const FHealthPtrMap& Map);

	/**
	 * Checks if at least one Health component is alive
	 * @param	Map	To check for alive components
	 * @returns	True if any still alive, false if not.
	 */
	static bool IsAnyAlive(const FHealthMap& Map);

#pragma endregion

#pragma region Utils
public:

	/** Gets main mesh */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, BlueprintPure, Category = "Data")
	USkeletalMeshComponent* GetMainMesh() const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data")
	void GetAllTags(TArray<FName>& Tags) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Data")
	void GetAllHealths(TArray<FHealth>& Healths, bool bIncludeMain = true) const;

	/**
	 * Finds the Health component that has the name Tag
	 * @param 	TargetTag			  	Tag name of the health component to search for.
	 * @param 	ReturnMainWhenNotFound	(Optional) When no component contains the TargetTag specifies
	 * 									if true will return main instead of null.
	 * @returns	The component that matches TargetName.
	 */
	FHealth* FindHealthDataByTag(const FName& TargetTag, const bool ReturnMainWhenNotFound = false);

	/**
	 * Finds the Health component and it's Defense that has the name Tag
	 * @param 	TargetTag			  	Tag name of the health component to search for.
	 * @param 	ReturnMainWhenNotFound	(Optional) When no component contains the TargetTag specifies
	 * 									if true will return main instead of null.
	 * @returns	The component pair that matches TargetName.
	 */
	FHealthPtrPair FindHealthDataByTagPair(const FName& TargetTag, const bool ReturnMainWhenNotFound = false);

#pragma endregion

};

