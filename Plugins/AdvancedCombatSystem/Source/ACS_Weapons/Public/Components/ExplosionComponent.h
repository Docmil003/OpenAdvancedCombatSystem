//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ACS_CoreElements.h"
#include "ExplosionComponent.generated.h"

struct FDamageInfo;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExploded);

/*
* A component class that implements the default behaviour to simulate an explosion damage
*/
UCLASS(ClassGroup=(AdvancedCombatSystem), meta=(BlueprintSpawnableComponent))
class ACS_WEAPONS_API UExplosionComponent : public USceneComponent
{
	GENERATED_BODY()
	
public:

	//Sets default values for this component's properties
	UExplosionComponent();
	
#pragma region Delegates

public:

	//Called after all explosion logic
	UPROPERTY(BlueprintAssignable, Category = "Explosion")
	FExploded OnExplosionSuccess;
	
	//Called when tried to explode but didn't for some reason
	UPROPERTY(BlueprintAssignable, Category = "Explosion")
	FExploded OnExplosionFailed;
	
#pragma endregion

#pragma region Autodestruction

public:

	//Specifies if should destroy this component's owner after the explosion. DO NOT USE IF INSIDE A POOLED BULLET
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Self-reaction")
	uint8 bDestroyOwner:1 = true;
	
	//Specifies if should destroy this component after the explosion. DO NOT USE IF INSIDE A POOLED BULLET
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Self-reaction", meta = (EditCondition = "!bDestroyOwner"))
	uint8 bDestroySelf:1 = true;
	
	//Indicates if should be reactivated again after exploded
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Self-reaction", meta = (EditCondition = "!bDestroyOwner && !bDestroySelf"))
	uint8 bReactiveAfterExplode:1 = true;
	
	//Indicates the delay until auto-activation
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Self-reaction", meta = (EditCondition = "!bDestroyOwner && !bDestroySelf && bReactiveAfterExplode"))
	float ReactivationDelay = 2;
	
	virtual void PostExplosion();
	
#pragma endregion

#pragma region Affecting

public:

	//Radius of the explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	float SphereOverlapRadius = 400;
	
	//Indicates if the owner should not be affected by this component's explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	uint8 bIgnoreOwner:1 = true;
	
	//Array of type object to query
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	
	//Array of all classes that should not be affected by the explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query", AdvancedDisplay)
	TArray<TSubclassOf<AActor>> ClassesToIgnore;
	
	//Class to exclusive filter in the explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	TSubclassOf<AActor> SpecificClassToFilter;
	
	//Simulates the explosion logic
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Explosion")
	void Explode(TArray<FActorDamageInfo>& DamageCaused);
	
#pragma endregion

#pragma region Damage

public:

	//Max damage that generates the explosion. Sended damaged can be greater if using a DamageCurve with points higher than 1
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	double MaxDamage = 200;
	
	//Type of damage sent by the explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage")
	TEnumAsByte<EDamageType> DamageType;
	
	/*
	*Curve to control the damage scale along the distance. 
	*Distance will be normalized to this curve's length, so the start range of the curve will be the closest and the end the max distance of affectance.
	*/
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage|Bias")
	TWeakObjectPtr<UCurveFloat> DamageCurve;
	
	//A 2 element vector which the X is used as damage bias from the outside while, the Y woll bias the core explosion
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Damage|Bias")
	FVector2D DamageRadiusBias;
	
	/*
	* Calculates tje damage fpr tje actor
	* @param Other Os the actor that will suffer the damage
	* @param Normalized The normal value of the damage (0-1)
	* @return The damage received (Normalized*Max)
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage")
	double CalculateDamage(const AActor* Other, double& Normalized) const;
	
	double CalculateDamage(const AActor* Other) const;
	
	//Builds damage for the specific victim
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Damage")
	FDamageInfo BuildDamageInfo(const AActor* Victim) const;
	
protected:

	virtual void DamageArea(TArray<FActorDamageInfo>& DamageCaused);

#pragma endregion

#pragma region Debug

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	uint8 bDrawDebugSphere:1 = false;
	
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Debug")
	void DrawDebugExplosion();
	
#pragma endregion

};