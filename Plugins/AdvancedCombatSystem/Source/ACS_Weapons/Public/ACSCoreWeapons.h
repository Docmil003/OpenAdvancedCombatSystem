// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectWrapper.h"
#include "Animation/AnimMontage.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "ACSCoreWeapons.generated.h"

#define ACS_WEAPONS_LOG AdvancedCombatSystem_Weapons_Log
DECLARE_LOG_CATEGORY_EXTERN(AdvancedCombatSystem_Weapons_Log, Log, All);

//Possible querys for trace
UENUM(BlueprintType)
namespace EQueryShape
{
	enum Type {
		Line,
		Sphere,
		Box
	};
}

//Method of triggering
UENUM(BlueprintType)
namespace ETriggerMode {
	enum Type {
		Single,		//Fires once
		Burst,		//Fires an specific amount of times with a delay between them
		Automatic	//Keeps firing until cancel or failed with a delay between triggers
	};
}

//Tells when to run part of code
UENUM(BlueprintType)
namespace EExecutionTiming {
	enum Type {
		Begin,	//Execute inmediatly
		End,	//Wait until all functionality ends
		Custom	//Execute it manualy
	};
}

//Method to check the collision
UENUM(BlueprintType)
namespace ETracingMethod {
	enum Type {
		Channel,	//Chanel collision
		Objects,	//Collision for objects
		Profile		//Collision by profile
	};
}

//Impact response types
UENUM(BlueprintType)
namespace EImpactResponse {
	enum Type {
		Block,		//Will block the bullet and invalidate it
		Ricochete,	//Bullet will bounce of the surface
		Penetrate,	//Bullet will penetrate the object
		Ignore		//Bullet will completly ignore the object
	};
}

//Method to use the pool system
UENUM(BlueprintType)
namespace EPoolingMethod {
	enum Type
	{
		None,		//Bullets will spawn and destroy normally, just like if it wasn't pooled. 
		Fixed,		//Reserves an specific amount of bullets to exist in the world. If there are not enough bullets in reserve will not spawn. If you use this method you can calculate the max amount possible by this operation: (Bullet max life active) * (Bullets per shot) * (Fire rate)
		Dynamic,	//Won't reserve any amount of bullets. Will spawn when needed and instead of despawn, will deactivate the bullet and reactivated when required. Bullets have a fixed inactive time, that when exeeded will automaticly despawn. It sorts the bullets to use a LIFO (Last input first output) algorithm to allow bullets to clear from memory if the shot rate decreases
		Hybrid		//Mixes the reserved amount from the fixed with the flexibility of the dynamic which results in if there's no bullet in reserve will spawn a new bullet. When Pool is over the required amount bullets will despawn to match the pool requirement
	};
}

//Struct of anim of the character and its weapon
USTRUCT(BlueprintType)
struct FCharWeaponAnim
{
	GENERATED_BODY()
public:

	//Montage for the character
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Character animation"))
	TObjectPtr<UAnimMontage> Character;

	//Animation for the weapon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Weapon animation"))
	TObjectPtr<UAnimSequenceBase> Weapon;

	//Animation rate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Play Rate"))
	double PlayRate = 1;

};

//Struct of anim of the character and its weapon (Soft referenced)
USTRUCT(BlueprintType)
struct FSoftCharWeaponAnim
{
	GENERATED_BODY()
public:

	//Montage for the character
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Character animation"))
	TSoftObjectPtr<UAnimMontage> Character;

	//Animation for the weapon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Weapon animation"))
	TSoftObjectPtr<UAnimSequenceBase> Weapon;

	//Animation rate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Play Rate"))
	double PlayRate = 1;

};

//Array of FCharWeaponAnim
USTRUCT(BlueprintType)
struct FCharWeaponAnimArray
{
	GENERATED_BODY()
public:

	//Array of CharWeaponAnim
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Anims"))
	TArray<FCharWeaponAnim> Anims;

};

//Array of FSoftCharWeaponAnim
USTRUCT(BlueprintType)
struct FSoftCharWeaponAnimArray
{
	GENERATED_BODY()
public:

	//Array of SoftCharWeaponAnim
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils", meta = (DisplayName = "Anims"))
	TArray<FSoftCharWeaponAnim> Anims;

};

//Array of FCharWeaponAnim
USTRUCT(BlueprintType)
struct FCharWeaponAnimArrayForIterations
{
	GENERATED_BODY()
public:

	//Array of CharWeaponAnim
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils")
	FCharWeaponAnim Start;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils")
	FCharWeaponAnim Loop;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Animation|Utils")
	FCharWeaponAnim End;

};

//Ricochet settings
USTRUCT(BlueprintType)
struct FBulletRicochet
{
	GENERATED_BODY()

public:
	
	//Indicates if it can ricochet
	UPROPERTY()
	uint8 Ricochets:1 = true;
	
	//Random deviaton on the reflection
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Ricochet", meta = (DisplayName = "RandomReflectedAngle"))
	double RandomDeflectedAngle = 10;
	
	//Indicates if can bounce an undetermined amount of times
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Ricochet", meta = (DisplayName = "InfiniteRicochette"))
	uint8 bInfiniteRicochet:1 = false;
	
	//Probability of the ricochet to happen. Only when using Infinite Ricochette
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Ricochet", meta = (DisplayName = "ReflectionProb", EditCondition="bInfiniteRicochet", EditConditionHides, ClampMin=0, ClampMax=100, UIMin=0, UIMax=100, Units="%"))
	double DeflectionProbability = 0.1;
	
	//This array follows the current bounce index for choose which probability should pick to ricochete. 
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Ricochet", meta = (DisplayName = "ReflectionProb", EditCondition = "!bInfiniteRicochet", EditConditionHides, ClampMin = 0, ClampMax = 100, UIMin = 0, UIMax = 100))
	TArray<double> DeflectionProb = TArray<double>();
	
	//% of speed reduction when bounce
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Ricochet", meta = (DisplayName = "PenetrationSpeedDecrement", UIMin = 0, UIMax = 100, Units = "%"))
	double SpeedDecrement = 20;

};

//Penetration settings
USTRUCT(BlueprintType)
struct FBulletPenetration
{
	GENERATED_BODY()

public:

	//Scales the thickness of the surface
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Penetration", meta = (DisplayName = "Hardness"))
	double Hardness = 1;

	//Deviation angle of the bullet on the impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Penetration", meta = (DisplayName = "RandomDeviation", UIMin = 0, UIMax = 180, Units = "deg"))
	double RandomDeviation = 20;

	//Probability to penetrate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Penetration", meta = (DisplayName = "PenetrationProb", ClampMin = 0, ClampMax = 1, UIMin = 0, UIMax = 1, Units = "%"))
	double PenetrationProb = 0.5;

	//Speed % reduction per each meter (100 unreal units)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Penetration", meta = (DisplayName = "PenetrationSpeedDecrement", UIMin = 0, UIMax = 100, Units = "%"))
	double PenetrationSpeedDecrement = 10;
	
};

//Hit reaction selection
USTRUCT(BlueprintType)
struct FBulletImpactReaction
{
	GENERATED_BODY()
public:

	//Reaction to the impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Impact", meta = (DisplayName = "Impact Reaction", MakeStructureDefaultValue = "Block"))
	TEnumAsByte<EImpactResponse::Type> ImpactSolver = EImpactResponse::Block;

	//Ricochet settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Impact", meta = (DisplayName = "Ricochete", EditCondition ="ImpactSolver==EImpactResponse::Ricochete", EditConditionHides))
	FBulletRicochet Ricochet;

	//Penetration settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Impact", meta = (DisplayName = "Penetration", EditCondition = "ImpactSolver==EImpactResponse::Penetrate", EditConditionHides))
	FBulletPenetration Penetration;
};

//Instructions for the bullet to follow
USTRUCT(BlueprintType)
struct FBulletSettings
{
	GENERATED_BODY()

public:

#pragma region Bullet

	//Velocity that will have at the start
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet")
	double InitialSpeed = 5000;

#pragma endregion

#pragma region Query

	//Radius of the trace. If 0 (or less) will cast a line trace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query", meta = (ClampMin = 0, UIMin=0, UIMax=10))
	double SphereRadius = 5;

	//Indicates if should trace against complex collision instead of simple
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query")
	uint8 TraceComplex:1 = false;

	//Method to trace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query")
	TEnumAsByte<ETracingMethod::Type> TraceMethod = ETracingMethod::Channel;

	//Channel used to query the collision on the movement
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query", meta=(EditCondition="TraceMethod==ETracingMethod::Channel", EditConditionHides))
	TEnumAsByte<ETraceTypeQuery> TraceChannel = TraceTypeQuery1;

	//Objects types used to query the collision on the movement
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query", meta = (EditCondition = "TraceMethod==ETracingMethod::Objects", EditConditionHides))
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectsTypes;

	//Profile name used to query the collision on the movement
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query", meta = (EditCondition = "TraceMethod==ETracingMethod::Profile", EditConditionHides))
	FName ProfileName;

	//List of classes to ignore the collision with
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Query")
	TArray<TSubclassOf<AActor>> IgnoreClasses;

#pragma endregion

#pragma region Direction

	//Direction (in spawn local space) to move the bullet
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction", AdvancedDisplay)
	FVector LocalDirection = FVector::ForwardVector;

	//Indicates if should align owner actor to face the velocity
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction", AdvancedDisplay)
	uint8 AlignActor:1 = true;

	//Component to aim to
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction")
	TObjectPtr<USceneComponent> HomingComponent;

	//Position to aim to
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction")
	FVector TargetPostion = FVector::Zero();

	//Indicates if the homming direction goes by a turn base or by changing speed vector.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction")
	uint8 bHommingByRotation:1 = true;

	//Strength of the redirection towards the target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction")
	float HomingDirectionSpeed = 1;

	//Strength of the redirection towards the target
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Direction")
	float HomingDirectionSpeedScale = 1;

#pragma endregion

#pragma region World Physics

	//Map of the reactions to each surface
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	TMap<TEnumAsByte<EPhysicalSurface>, FBulletImpactReaction> SurfaceReaction = TMap<TEnumAsByte<EPhysicalSurface>, FBulletImpactReaction>();
	
	//If surface was not found, will use this reaction
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	FBulletImpactReaction DefaultSurfaceReaction;

	//Main distance that can penetrate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics", meta=(Unit = "cm"))
	float Resistance = 200;

	//Mass of the bullet. Acts as scale of the gravity
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	double Mass = 0.01;

	//Gravity for the bullet
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	FVector Gravity = FVector(0, 0, -986);

	//Indicates if the gravity acumulated persist through frames, otherwise, only will apply current frame's gravity
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	uint8 bGatherGravity:1 = true;

	//Resistance of the air, decelerates the bullet
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Bullet|Physics")
	double AirResistance = 0.05;

#pragma endregion

};

//Struct containing information about a modification
USTRUCT(BlueprintType)
struct FModificatorInfo
{
	GENERATED_BODY()

public:

	//Name of the modification
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Modification")
	FText ModName;

	//Index of the modification
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon|Modification")
	uint8 ModIndex;

	FModificatorInfo() {}

	FModificatorInfo(FText Name, uint8 ModIndex) : ModName(Name), ModIndex(ModIndex) {}

};

//Holds a reference to a pointer and handles the pointer destruction, depending in if was add as dynamic or static
template <typename T>
USTRUCT()
struct FSimplePtr
{

private:

	//Pointer that holds the reference to the variable
	T* Ptr;

	//Indicate if the pointer is dynamic or static
	uint8 bDynamic:1;

public:

	FSimplePtr() : Ptr(nullptr), bDynamic(false){}
	~FSimplePtr() { Clear(); }

	//Removes and destroy if needed the pointer
	void Clear() {
		if (!Ptr) return;
		if (bDynamic) delete Ptr;
		Ptr = nullptr;
		bDynamic = false;
	}

	//Clears the value and updates it to a static allocated variable
	void SetPtrStatic(T& Variable){
		Clear();
		Ptr = &Variable;
		bDynamic = false;
	}

	//Clears the value and updates it to a dynamic allocated variable
	void SetPtrDynamic(const T Variable) {
		Clear();
		Ptr = new T(Variable);
		bDynamic = true;
	}

	//Indicates if pointing to a dynamic pointer
	bool IsDynamic() const { return bDynamic; }

	FSimplePtr operator=(const FSimplePtr& Other) {
		if (this != &Other) return *this;
		Clear();
		Ptr = Other.Ptr;
		bDynamic = Other.bDynamic;
		return *this;
	}

	bool operator==(const FSimplePtr& Other) {
		return Ptr == Other.Ptr;
	}

	T& operator *(){ return *Ptr; }

	operator T*() const { return Ptr; }

	operator bool() const { return Ptr != nullptr; }

};

