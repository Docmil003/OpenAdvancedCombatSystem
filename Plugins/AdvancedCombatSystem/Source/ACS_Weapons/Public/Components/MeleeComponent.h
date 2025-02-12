//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "ACSCoreWeapons.h"
#include "Kismet/KismetSystemLibrary.h"
#include "MeleeComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FHitEvent, const FHitResult&, Hit, AActor*, HitActor, UActorComponent*, HitComp);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMultiHitEvent, TArray<FHitResult>, Hit, AActor*, HitActor);

typedef TIndexedContainerIterator<TArray<FHitResult>, FHitResult, int32> THitIterator;

/*
* A component class that traces to check collision to interpret as hit
*/
UCLASS(ClassGroup = (AdvancedCombatSystem), Blueprintable, meta = (BlueprintSpawnableComponent))
class ACS_WEAPONS_API UMeleeComponent : public USceneComponent
{
	GENERATED_BODY()
	
	//Set default values for this componen's properties
	UMeleeComponent();
	
#pragma region Inherance
public:

	//Activates the collision query
	virtual void Activate(bool bReset) override;
	
	//Deactivates the collision query
	virtual void Deactivate() override;
	
	//Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
#pragma endregion

#pragma region Internal data
protected:

	//Actors to ignore during the execution
	UPROPERTY(BlueprintReadWrite, Category = "Query|Ignore")
	TArray<AActor*> ActorsToIgnore;
	
	UPROPERTY(BlueprintReadWrite, Category = "Query|Debug")
	TEnumAsByte<EDrawDebugTrace::Type> DebugDraw;
	
	//Gets the source component used for get the hit area
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Source")
	virtual FORCEINLINE USceneComponent* GetSource() { return bUseParentInfo ? GetAttachParent() : this; }

	virtual const FORCEINLINE USceneComponent* GetSourceConst() const { return bUseParentInfo ? GetAttachParent() : this; }
	
private:

	//Location from where previous frame ended
	FVector PreviousLoc;
	
	//Number of hitted actors
	int32 HitActors;
	
#pragma endregion

#pragma region Delegates

public:

	//Called when collides with something
	UPROPERTY(BlueprintAssignable, Category = "Hit")
	FHitEvent OnHit;
	
	//Called when hits a blocking object
	UPROPERTY(BlueprintAssignable, Category = "Hit")
	FHitEvent OnBlocked;
	
	//Called when hits an objects multiple times in a same frame
	UPROPERTY(BlueprintAssignable, Category = "Hit")
	FMultiHitEvent OnMultiHit;
	
#pragma endregion
	
#pragma region Tracing
public:

	//Method to use for the traces
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	TEnumAsByte<ETracingMethod::Type> TraceMethod;
	
	//Indicates if the actor hitted should be added to the ignore list until restarted or deactivate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	bool bHitOnce = true;
	
	//Shape used for the collision query
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	TEnumAsByte<EQueryShape::Type> QueryShape = EQueryShape::Sphere;
	
	//Trace channel for the trace
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query", meta = (EditCondition = "TraceMethod==ETracingMethod::Channel", EditConditionHides))
	TEnumAsByte<ETraceTypeQuery> TraceChannel;
	
	//Array of object type query to hit
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query", meta = (EditCondition = "TraceMethod==ETracingMethod::Objects", EditConditionHides))
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectsTypes;
	
	//Profile for the trace hit
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query", meta = (EditCondition = "TraceMethod==ETracingMethod::Profile", EditConditionHides))
	FName ProfileName;
	
	//Radius of the sphere when no using parent info
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source", meta = (EditCondition = "!bUseParentInfo && QueryShape==EQueryShape::Sphere"))
	float SphereRadius = 20;
	
	//Extent of the box when no using parent info
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Source", meta = (EditCondition = "!bUseParentInfo && QueryShape==EQueryShape::Box"))
	FVector BoxExtent = FVector(20, 20, 50);
	
	//List of classes to ignore
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hit")
	TArray<TSubclassOf<AActor>> ClassesToIgnore;
	
	//Direction to use when previous location is too similar to the new one
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query", AdvancedDisplay)
	FVector FallBack = FVector(1, 0, 0);
	
	//Min distance between previous hit and new before using fallback
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	float MinDistance = 1;
	
	//Executes the trace from previous location to current
	UFUNCTION(BlueprintCallable, Category = "Query")
	virtual bool TraceCast(TArray<FHitResult>& Results) const;
	
#pragma endregion

#pragma region Interactions

	//Array of surfaces where the impact should be blocked
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hit")
	TArray<TEnumAsByte<EPhysicalSurface>> BlockingSurfaces;
	
	//Indicates if can hit a same actor more than once in the collision query under the same execution
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Query")
	bool bCanHitSameMultipleTimesAtOneTrace = false;
	
	//Indicates if the MultiHitDelegate will be executed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hit")
	bool bGenerateMultiHitEvent = false;
	
	//Indicates if the hit bound data should be received from it's parent's bounds, otherwise will be self indicated
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Hit")
	bool bUseParentInfo = true;
	
#pragma endregion

#pragma region Utils
public:

	/* 
	* Resets the array of actors to ignore.
	* @param bDeep is slower but will update the list calling and appending from get all actors of class for each class to ignore so will have the newest data
	*/
	UFUNCTION(BlueprintCallable, Category = "Query")
	void ResetActorsToIgnore(bool bDeep);
	
	//Generates and handles the hit
	void HitQuery();
	
	/*
	* Indicates if the same actor appears multiple times in the Iteration list.
	* @param It Iteration list from which to seach for the first actors repetition. THE ITERATION MUST BE SORTED
	* @param It Returns the Hit results of each time the first actor of the iteration has appeared
	* @return If the first actor of the Iterations appears more than once
	*/
	bool HitsSameManyTimes(THitIterator& It, TArray<FHitResult>& Hits);
	
#pragma endregion

#pragma region Getters
public:

	//Gets the begin  position of the trace at this frame (End position of previous frame)
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FVector GetStart() const;
	
	//Gets the end position of the trace at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FORCEINLINE FVector GetEnd() const { return GetSourceConst()->GetComponentLocation(); }
	
	//Gets the half size for the box trace at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FORCEINLINE FVector GetHalfSize() const { return bUseParentInfo ? GetAttachParent()->GetLocalBounds().BoxExtent : BoxExtent;}
	
	//Gets the orientation for the box trace at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FORCEINLINE FRotator GetOrientation() const { return GetSourceConst()->GetComponentRotation();}
	
	//Gets the radius for the sphere trace at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FORCEINLINE double GetRadius() const { return bUseParentInfo ? GetAttachParent()->GetLocalBounds().SphereRadius : SphereRadius; }
	
	//Gets the forward vector of the component at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FORCEINLINE FVector GetSourceForward() const { return GetSourceConst()->GetForwardVector();}
	
	//Gets the trace length at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	float GetSourceFrameDistance() const;
	
	//Gets a valid position for previous location at this frame
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Query")
	FVector GetFixedPreviousLocation() const;

#pragma endregion

};
