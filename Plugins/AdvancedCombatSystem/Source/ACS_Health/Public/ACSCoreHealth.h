// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "Engine/TimerHandle.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ACSCoreHealth.generated.h"

#define ACS_HEALTH_LOG	AdvancedCombatSystem_Health_Log
DECLARE_LOG_CATEGORY_EXTERN(AdvancedCombatSystem_Health_Log, Log, All);

enum EDamageType : int;
class UHealthComponent;
struct FDamageInfo;

//Method to absorb the damage by the armor
UENUM(BlueprintType)
enum class EArmordefenseMethod : uint8
{
	FirstFound,			//No pre-sort, will take the first non-broken armor piece. If there's damage unabsorbed due to a to strong damage or its settings, will get the next armor until none remains.
	Mostdefense,		//Works like "FirstFound", but sorts the armor by greatest absorbs damage, so it's order by the most defensive
	DivideThroughAll,	//Divides damage to affect all armor, remaining damage will be send to main health
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnDamaged, FHealth*, FDamageInfo);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealed, FHealth*, FHealInfo);

//Struct-based component that handles health
USTRUCT(BlueprintType)
struct FHealth
{
	GENERATED_BODY()

	friend class UHealthComponent;

private:
	//Current amount of health
	TSharedRef<double> Health = TSharedRef<double>(new double());

	//Max health
	TSharedRef<double> MaxHealth = TSharedRef<double>(new double());

public:

	//Identifier name for the health
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health", meta = (DisplayName = "Health Name"))
	FName Tag;

	//Health that starts with by default
	UPROPERTY(EditAnywhere, Category = "Health")
	double InitialHealth = 100;

	//Max health that starts with by default
	UPROPERTY(EditAnywhere, Category = "Health")
	double InitialMaxHealth = 100;

	//Indicates if the health can go above max health
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health|Overdose")
	bool bCanOverdose = false;

	//Limit to the overdose (if 0 or less will assume as unlimited)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health|Overdose", meta = (EditCondition = "bCanOverdose", EditConditionHides))
	double OverdoseLimit = 0;

	//Amount of health decrease per second when overhealed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health|Overdose", meta = (EditCondition = "bCanOverdose", EditConditionHides))
	double OverdoseDecrease = 0;

	//Delay to start the overhealth drain
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health|Overdose", meta = (EditCondition = "bCanOverdose", EditConditionHides))
	double OverdoseReductionDelay = 2;
	
	//Indicates if should make another delay when received more health when already overdosed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Health|Overdose", meta=(DisplayName="Restart Delay When Rerunneth Over", EditCondition = "bCanOverdose", EditConditionHides))
	bool bDelayWhenRehealed  = true;

	//Tick to drain health
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Health|Overdose", meta = (EditCondition = "bCanOverdose", EditConditionHides))
	double OverdoseTick = 0.03;

	bool bMainHealth = false;

	//Handle for the overdose health
	FTimerHandle OverdoseHandle;

	class UHealthComponent* OwnerHealthComp;

	bool IsOverdosed() const { return *Health > *MaxHealth; }
	const double* GetHealth() const { return &*Health; }
	const double* GetMaxHealth() const { return &*MaxHealth; }

	operator bool() const { return *Health > 0; }

	bool operator== (const FHealth& Other) const
	{
		return this == &Other;
	}

	bool operator== (const FName& OtherTag) const
	{
		return this->Tag.IsEqual(OtherTag);
	}

	friend uint32 GetTypeHash(const FHealth& Other)
	{
		return GetTypeHash(Other.Tag);
	}

	FHealth operator=(const FHealth& Other)
	{
		if (this == &Other) return *this;

		Tag = Other.Tag;
		*Health = *Other.Health;
		*MaxHealth = *Other.MaxHealth;
		bCanOverdose = Other.bCanOverdose;
		OverdoseLimit = Other.OverdoseLimit;
		OverdoseDecrease = Other.OverdoseDecrease;
		OverdoseReductionDelay = Other.OverdoseReductionDelay;
		bDelayWhenRehealed = Other.bDelayWhenRehealed;
		OverdoseTick = Other.OverdoseTick;
		OverdoseHandle = Other.OverdoseHandle;
		bMainHealth = Other.bMainHealth;

		return *this;
	}

	//Constructor that copies the other FHealth data, the health and max health is set to the same value, it wont be shared
	FHealth(const FHealth& Other) : Tag(Other.Tag),	bCanOverdose(Other.bCanOverdose), OverdoseLimit(Other.OverdoseLimit), 
		OverdoseDecrease(Other.OverdoseDecrease), OverdoseReductionDelay(Other.OverdoseReductionDelay), bDelayWhenRehealed(Other.bDelayWhenRehealed),
		OverdoseTick(Other.OverdoseTick), OverdoseHandle(Other.OverdoseHandle)
	{
		*Health = *Other.Health;
		*MaxHealth = *Other.MaxHealth;
	}

	FHealth() : Tag("None"), InitialHealth(100), InitialMaxHealth(100), bCanOverdose(false), OverdoseLimit(100), OverdoseDecrease(10),
		OverdoseReductionDelay(3), bDelayWhenRehealed(true), OverdoseTick(0.033)
	{
		*Health = InitialHealth;
		*MaxHealth = InitialMaxHealth;
	}

};

/*
* Struct with the damage information.
* The defense indicates the percentage of damage that will absorb. The remaining damage will be ignored and sent to the next health component. Remember it combines the per type with the per bone, or if one is not found with the other, and is none of both are found, will use the default.
* The Scale damage if a percentage of damage taken 0 been none, 100 the base and 200 the double. Will apply the scaled damage to itself. Remember it combines the per type with the per bone, or if one is not found with the other, and is none of both are found, will use the default.
* Here are a few examples with the same damage (of 100pts) and only changing absorbption and scale: 
* 
*	|	Absorbption	|	Taken(To scale)	|	Ignored(for next comp)	|	Scale	|	Final		|
*	|		50	%	|		50	pts		|			50	pts			|	50	%	|	25		pts	|
*	|		75	%	|		75	pts		|			25	pts			|	25	%	|	18,75	pts	|
*	|		100	%	|		100	pts		|			0	pts			|	50	%	|	50		pts	|
*	|		25	%	|		25	pts		|			75	pts			|	200	%	|	50		pts	|
*	|		0	%	|		0	pts		|			100	pts			|	100	%	|	0		pts	|
*	|		90	%	|		90	pts		|			10	pts			|	0	%	|	0		pts	|
* 
*/
USTRUCT(BlueprintType)
struct FDefenseProperties 
{
	GENERATED_BODY()

public:

	//Absorbption tag
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense", meta = (DisplayName = "Defense Name"))
	FName Tag;

	//Default Defense of the damage, used when no other settings found.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Absorbtion", meta = (UIMin = 0, UIMax = 100, Units="%"))
	double DefaultDamageAbsorbption = 100;

	//Default damage scale, used when no other settting found.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Scale", meta = (UIMin = 0, UIMax = 200, Units = "%"))
	double DefaultScaleDamage = 100;

	//defense percentage based on the damage type
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Absorbtion", meta = (UIMin = 0, UIMax = 100))
	TMap<TEnumAsByte<EDamageType>, double> DamageTypeAbsorbtion;

	//Damage scale based on the damage type
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Scale", meta = (UIMin = 0, UIMax = 200))
	TMap<TEnumAsByte<EDamageType>, double> ScaleDamagePerType;

	//defense percentage based on the damaged bone
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Absorbtion", meta = (UIMin = 0, UIMax = 100))
	TMap<FName, double> DamageBoneAbsorbption;

	//Damage scale based on the damaged bone
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Defense|Scale", meta = (UIMin = 0, UIMax = 100))
	TMap<FName, double> ScaleDamagePerBone;

	FDefenseProperties operator=(const FDefenseProperties& Other)
	{
		if (this == &Other) return *this;

		Tag = Other.Tag;
		DefaultDamageAbsorbption = Other.DefaultDamageAbsorbption;
		DefaultScaleDamage = Other.DefaultScaleDamage;
		DamageTypeAbsorbtion = Other.DamageTypeAbsorbtion;
		ScaleDamagePerType = Other.ScaleDamagePerType;
		DamageBoneAbsorbption = Other.DamageBoneAbsorbption;
		ScaleDamagePerBone = Other.ScaleDamagePerBone;

		return *this;
	}

	FDefenseProperties(){}

	FDefenseProperties(const FDefenseProperties* Other) : FDefenseProperties(*Other){}

	FDefenseProperties(const FDefenseProperties& Other) : Tag(Other.Tag), DefaultDamageAbsorbption(Other.DefaultDamageAbsorbption),
		DefaultScaleDamage(Other.DefaultScaleDamage), DamageTypeAbsorbtion(Other.DamageTypeAbsorbtion),
		ScaleDamagePerType(Other.ScaleDamagePerType), DamageBoneAbsorbption(Other.DamageBoneAbsorbption),
		ScaleDamagePerBone(Other.ScaleDamagePerBone)
	{
	}

};

//Struct with the healing information
USTRUCT(BlueprintType)
struct FHealInfo
{
	GENERATED_BODY()
public:

	//Tag to identify the of heal received
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heal")
	FName Tag;

	//Component's tag that aims to heal
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heal")
	FName TargetTag = "Main";

	//Indicates if recovered health is determined in raw values. Otherwise will represent a percentage of the max health.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heal")
	bool bBruteHealth = true;

	//Amount of health to recover
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heal", meta=(EditCondition = "bBruteHealth"))
	double Health = 0;

	//Percentage of recovered health
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Heal", meta = (EditCondition = "!bBruteHealth", UIMin=0, UIMax=100, Units = "Percent"))
	double HealPercentage = 100;
	
};

UCLASS()
class ACS_HEALTH_API UACSHealthUtilsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	//Gets the current health of a FHealth
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	static double GetHealth(const FHealth& Health) { return *Health.GetHealth(); }

	//Gets the current max health of a FHealth
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
	static double GetMaxHealth(const FHealth& Health) { return *Health.GetMaxHealth(); }

	

};