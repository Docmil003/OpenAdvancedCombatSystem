// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.



#pragma once



#include "CoreMinimal.h"

#include "ACS_CoreElements.h"

#include "Components/ActorComponent.h"

#include "ConstantDamagerComponent.generated.h"



struct FDamageInfo;



USTRUCT(BlueprintType)

struct FDamageSendData {



	GENERATED_BODY()



public:



	//Infomation about the damage to send

	UPROPERTY(BlueprintReadWrite, Category = "Damage")

	FDamageInfo Damage = FDamageInfo();



	//Indicate if lifetime

	UPROPERTY(BlueprintReadWrite, Category = "Lifetime")

	bool bUndefinedLifetime = true;



	//Time that the damage will last

	UPROPERTY(BlueprintReadWrite, Category = "Lifetime", meta = (EditCondition = "!bUndefinedLifetime"))

	float DamageLifetime = 0;



	//Indicate if should override the damage sender as this owner's

	UPROPERTY(BlueprintReadWrite, Category = "Damage")

	bool SelfCauser = false;



	//Indicates the name of the component to damage specifically

	UPROPERTY(BlueprintReadWrite, Category = "Damage")

	FName SpecificTarget = FName("Main");



};



/*
* A component ready to constantly damage the owner on each tick
*/

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class ACS_HEALTH_API UConstantDamagerComponent : public UActorComponent

{

	GENERATED_BODY()



public:	

	// Sets default values for this component's properties

	UConstantDamagerComponent();



#pragma region Core

protected:



	//Array of damage to send on each frame

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Damage")

	TArray<FDamageSendData> DamageDataCollection;



	// Called every frame

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;



	//Sends damage to the owner of this component

	UFUNCTION(BlueprintCallable, Category = "Damage")

	void Damage(const float DeltaTime);



#pragma endregion



#pragma region GETTERS

public:	



	//Gets an array with all the damaging types

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage")

	void GetDamageTypes(TArray<TEnumAsByte<EDamageType>>& DamageTypes) const;



	//Indicates if has a damage of the type passed per parameter

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Damage")

	bool HasDamageType(TEnumAsByte<EDamageType> DamageType) const;



	FORCEINLINE const TArray<FDamageSendData>& GetDamageDataCollection() { return DamageDataCollection; }



#pragma endregion



#pragma region SETTERS

public:	



	/*
	* Adds damage to generate each tick
	* @param Data Is the information that will be send to the owner on each tick
	* @param bForce Forces to add this damage type even if there's already a same type damage
	*/

	UFUNCTION(BlueprintCallable, Category = "Damage", meta=(AdvancedDisplay="bForce"))

	void AddDamageData(const FDamageSendData& Data, bool bForce = false);



	//Removes the damage info by index

	UFUNCTION(BlueprintCallable, Category = "Damage")

	void RemoveDamageByIndex(int32 Index);



	//Removes the damage information that sends this information type

	UFUNCTION(BlueprintCallable, Category = "Damage")

	void RemoveDamageByType(const TEnumAsByte<EDamageType> Type);



	//Removes the damage information that targets this component

	UFUNCTION(BlueprintCallable, Category = "Damage")

	void RemoveDamageByTarget(const FName& Target);



#pragma endregion



};

