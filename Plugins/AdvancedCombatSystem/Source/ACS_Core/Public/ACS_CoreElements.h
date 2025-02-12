//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "Components/PrimitiveComponent.h"
#include "BlueprintFileUtilsBPLibrary.h"
#include "ACS_CoreElements.generated.h"

/** Types of Damages in the game */
UENUM(BlueprintType)
enum EDamageType : int
{
	DamageType_Default UMETA(DisplayName = "Default"),
	DamageType1 UMETA(Hidden),
	DamageType2 UMETA(Hidden),
	DamageType3 UMETA(Hidden),
	DamageType4 UMETA(Hidden),
	DamageType5 UMETA(Hidden),
	DamageType6 UMETA(Hidden),
	DamageType7 UMETA(Hidden),
	DamageType8 UMETA(Hidden),
	DamageType9 UMETA(Hidden),
	DamageType10 UMETA(Hidden),
	DamageType11 UMETA(Hidden),
	DamageType12 UMETA(Hidden),
	DamageType13 UMETA(Hidden),
	DamageType14 UMETA(Hidden),
	DamageType15 UMETA(Hidden),
	DamageType16 UMETA(Hidden),
	DamageType17 UMETA(Hidden),
	DamageType18 UMETA(Hidden),
	DamageType19 UMETA(Hidden),
	DamageType20 UMETA(Hidden),
	DamageType21 UMETA(Hidden),
	DamageType22 UMETA(Hidden),
	DamageType23 UMETA(Hidden),
	DamageType24 UMETA(Hidden),
	DamageType25 UMETA(Hidden),
	DamageType26 UMETA(Hidden),
	DamageType27 UMETA(Hidden),
	DamageType28 UMETA(Hidden),
	DamageType29 UMETA(Hidden),
	DamageType30 UMETA(Hidden),
	DamageType31 UMETA(Hidden),
	DamageType32 UMETA(Hidden),
	DamageType33 UMETA(Hidden),
	DamageType34 UMETA(Hidden),
	DamageType35 UMETA(Hidden),
	DamageType36 UMETA(Hidden),
	DamageType37 UMETA(Hidden),
	DamageType38 UMETA(Hidden),
	DamageType39 UMETA(Hidden),
	DamageType40 UMETA(Hidden),
	DamageType41 UMETA(Hidden),
	DamageType42 UMETA(Hidden),
	DamageType43 UMETA(Hidden),
	DamageType44 UMETA(Hidden),
	DamageType45 UMETA(Hidden),
	DamageType46 UMETA(Hidden),
	DamageType47 UMETA(Hidden),
	DamageType48 UMETA(Hidden),
	DamageType49 UMETA(Hidden),
	DamageType50 UMETA(Hidden),
	DamageType51 UMETA(Hidden),
	DamageType52 UMETA(Hidden),
	DamageType53 UMETA(Hidden),
	DamageType54 UMETA(Hidden),
	DamageType55 UMETA(Hidden),
	DamageType56 UMETA(Hidden),
	DamageType57 UMETA(Hidden),
	DamageType58 UMETA(Hidden),
	DamageType59 UMETA(Hidden),
	DamageType60 UMETA(Hidden),
	DamageType61 UMETA(Hidden),
	DamageType62 UMETA(Hidden),
	DamageType_Max UMETA(Hidden)
};


/** Types of Weapons in the game */
UENUM(BlueprintType)
enum EWeapon : int
{
	Weapon_Default UMETA(DisplayName = "Default"),
	Weapon1 UMETA(Hidden),
	Weapon2 UMETA(Hidden),
	Weapon3 UMETA(Hidden),
	Weapon4 UMETA(Hidden),
	Weapon5 UMETA(Hidden),
	Weapon6 UMETA(Hidden),
	Weapon7 UMETA(Hidden),
	Weapon8 UMETA(Hidden),
	Weapon9 UMETA(Hidden),
	Weapon10 UMETA(Hidden),
	Weapon11 UMETA(Hidden),
	Weapon12 UMETA(Hidden),
	Weapon13 UMETA(Hidden),
	Weapon14 UMETA(Hidden),
	Weapon15 UMETA(Hidden),
	Weapon16 UMETA(Hidden),
	Weapon17 UMETA(Hidden),
	Weapon18 UMETA(Hidden),
	Weapon19 UMETA(Hidden),
	Weapon20 UMETA(Hidden),
	Weapon21 UMETA(Hidden),
	Weapon22 UMETA(Hidden),
	Weapon23 UMETA(Hidden),
	Weapon24 UMETA(Hidden),
	Weapon25 UMETA(Hidden),
	Weapon26 UMETA(Hidden),
	Weapon27 UMETA(Hidden),
	Weapon28 UMETA(Hidden),
	Weapon29 UMETA(Hidden),
	Weapon30 UMETA(Hidden),
	Weapon31 UMETA(Hidden),
	Weapon32 UMETA(Hidden),
	Weapon33 UMETA(Hidden),
	Weapon34 UMETA(Hidden),
	Weapon35 UMETA(Hidden),
	Weapon36 UMETA(Hidden),
	Weapon37 UMETA(Hidden),
	Weapon38 UMETA(Hidden),
	Weapon39 UMETA(Hidden),
	Weapon40 UMETA(Hidden),
	Weapon41 UMETA(Hidden),
	Weapon42 UMETA(Hidden),
	Weapon43 UMETA(Hidden),
	Weapon44 UMETA(Hidden),
	Weapon45 UMETA(Hidden),
	Weapon46 UMETA(Hidden),
	Weapon47 UMETA(Hidden),
	Weapon48 UMETA(Hidden),
	Weapon49 UMETA(Hidden),
	Weapon50 UMETA(Hidden),
	Weapon51 UMETA(Hidden),
	Weapon52 UMETA(Hidden),
	Weapon53 UMETA(Hidden),
	Weapon54 UMETA(Hidden),
	Weapon55 UMETA(Hidden),
	Weapon56 UMETA(Hidden),
	Weapon57 UMETA(Hidden),
	Weapon58 UMETA(Hidden),
	Weapon59 UMETA(Hidden),
	Weapon60 UMETA(Hidden),
	Weapon61 UMETA(Hidden),
	Weapon62 UMETA(Hidden),
	Weapon_Max UMETA(Hidden)
};

/** Types of Ammo in the game */
UENUM(BlueprintType)
enum EAmmoType : int
{
	AmmoType_Default UMETA(DisplayName = "Default"),
	AmmoType1 UMETA(Hidden),
	AmmoType2 UMETA(Hidden),
	AmmoType3 UMETA(Hidden),
	AmmoType4 UMETA(Hidden),
	AmmoType5 UMETA(Hidden),
	AmmoType6 UMETA(Hidden),
	AmmoType7 UMETA(Hidden),
	AmmoType8 UMETA(Hidden),
	AmmoType9 UMETA(Hidden),
	AmmoType10 UMETA(Hidden),
	AmmoType11 UMETA(Hidden),
	AmmoType12 UMETA(Hidden),
	AmmoType13 UMETA(Hidden),
	AmmoType14 UMETA(Hidden),
	AmmoType15 UMETA(Hidden),
	AmmoType16 UMETA(Hidden),
	AmmoType17 UMETA(Hidden),
	AmmoType18 UMETA(Hidden),
	AmmoType19 UMETA(Hidden),
	AmmoType20 UMETA(Hidden),
	AmmoType21 UMETA(Hidden),
	AmmoType22 UMETA(Hidden),
	AmmoType23 UMETA(Hidden),
	AmmoType24 UMETA(Hidden),
	AmmoType25 UMETA(Hidden),
	AmmoType26 UMETA(Hidden),
	AmmoType27 UMETA(Hidden),
	AmmoType28 UMETA(Hidden),
	AmmoType29 UMETA(Hidden),
	AmmoType30 UMETA(Hidden),
	AmmoType31 UMETA(Hidden),
	AmmoType32 UMETA(Hidden),
	AmmoType33 UMETA(Hidden),
	AmmoType34 UMETA(Hidden),
	AmmoType35 UMETA(Hidden),
	AmmoType36 UMETA(Hidden),
	AmmoType37 UMETA(Hidden),
	AmmoType38 UMETA(Hidden),
	AmmoType39 UMETA(Hidden),
	AmmoType40 UMETA(Hidden),
	AmmoType41 UMETA(Hidden),
	AmmoType42 UMETA(Hidden),
	AmmoType43 UMETA(Hidden),
	AmmoType44 UMETA(Hidden),
	AmmoType45 UMETA(Hidden),
	AmmoType46 UMETA(Hidden),
	AmmoType47 UMETA(Hidden),
	AmmoType48 UMETA(Hidden),
	AmmoType49 UMETA(Hidden),
	AmmoType50 UMETA(Hidden),
	AmmoType51 UMETA(Hidden),
	AmmoType52 UMETA(Hidden),
	AmmoType53 UMETA(Hidden),
	AmmoType54 UMETA(Hidden),
	AmmoType55 UMETA(Hidden),
	AmmoType56 UMETA(Hidden),
	AmmoType57 UMETA(Hidden),
	AmmoType58 UMETA(Hidden),
	AmmoType59 UMETA(Hidden),
	AmmoType60 UMETA(Hidden),
	AmmoType61 UMETA(Hidden),
	AmmoType62 UMETA(Hidden),
	AmmoType_Max UMETA(Hidden)
};

//Structure of the damage's information
USTRUCT(BlueprintType)
struct FDamageInfo
{
	GENERATED_BODY()
	
public:

	//Amount of raw generated damage
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	double Damage = 10;
	
	//Sender of the damage
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	TObjectPtr<AActor> Sender = nullptr;
	
	//Impact of the damage
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FVector Point = FVector::Zero();
	
	//Hit normal of the impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FVector Normal = FVector::Zero();

	//Damage's direction
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FVector Direction = FVector::Zero();

	//Specific hitted component
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;
	
	//Specific hitted bone
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FName Bone = FName();
	
	//Type of damage received
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	TEnumAsByte<EDamageType> Type = EDamageType::DamageType1;
	
	//When 0 or less will be consider as a "Fire and Forget" Damage. In case is more than 0 will be taken as a continuous damage, it represent the fraction of damage received
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	float DeltaDamage = -1;
	
	//Indicates if the source of the damage is ambient (heat, building collapse, cold, bees, etc) or artificial (fire weapons, melee weapons, etc)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	bool bEnviromental = false;
	
	//Any other data to share
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FJsonObjectWrapper Metadata = FJsonObjectWrapper();
	
	// FDamageInfo() : FDamageInfo(
	// 	10,  nullptr, FVector::Zero(), FVector::Zero(),
	// 	FVector::Zero(), nullptr, FName(), DamageType1,
	// 	-1, false, FJsonObjectWrapper()
	// ){};
	
	FDamageInfo(
		double Damage = 10,
		TObjectPtr<AActor> Sender = nullptr,
		FVector Point = FVector::Zero(),
		FVector Normal = FVector::Zero(),
		FVector Direction = FVector::Zero(),
		TObjectPtr<UPrimitiveComponent> HitComponent = nullptr,
		FName Bone = FName(),
		TEnumAsByte<EDamageType> Type = EDamageType::DamageType1,
		float DeltaDamage = -1,
		bool bEnviromental = false,
		FJsonObjectWrapper Metadata = FJsonObjectWrapper()
	) : 
		Damage(Damage),
		Sender(Sender),
		Point(Point),
		Normal(Normal),
		Direction(Direction),
		HitComponent(HitComponent),
		Bone(Bone),
		Type(Type),
		DeltaDamage(DeltaDamage),
		bEnviromental(bEnviromental),
		Metadata(Metadata)
	{};
	
};

USTRUCT(BlueprintType)
struct FActorDamageInfo
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	AActor* Victim;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	FDamageInfo Damage;
	
	FActorDamageInfo() : Victim(nullptr), Damage(FDamageInfo()){}
	
	FActorDamageInfo(AActor* Victim, const FDamageInfo& Damage) : Victim(Victim), Damage(Damage){}
	
	FActorDamageInfo operator=(const FActorDamageInfo& Other)
	{
		if(this == &Other) return *this;
		
		Victim = Other.Victim;
		Damage = Other.Damage;
		
		return *this;
	}
	
};

UCLASS()
class ACS_CORE_API UCoreHelperFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:

#if WITH_EDITOR

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static int GetNumOfActiveEWeapon();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static int GetNumOfActiveEDamage();
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static int GetNumOfActiveEAmmo();

#endif
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static FDamageInfo BuildDamageInfo(const FHitResult& Hit, double Damage, AActor* Sender, TEnumAsByte<EDamageType> Type, bool Ambiental, float DeltaTime = -1);
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static void GetClosestParentBoneInArray(const USkeletalMeshComponent* const Mesh, FName& Bone, const TArray<FName>& List);
	
	/**
	*Gets number of entries of an enum.
	@param Enum	The enum to check lenght.
	@returns The number of entries the enum contains.
	*/	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static int32 GetNumberOfEntries(const UEnum* Enum){return Enum->NumEnums();};
	
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Advanced Combat System|Utils")
	static FORCEINLINE AActor* GetDamagedActor(const FDamageInfo& Damage) { return Damage.HitComponent ? Damage.HitComponent->GetOwner() : nullptr;}
	
};