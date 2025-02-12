// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ACSCoreWeapons.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "FakeBulletComponent.generated.h"

struct FBulletRicochet;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FBlockingHit, const FHitResult&, Hit, const FVector&, DesiredOffset, AActor*, HitActor);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ACS_WEAPONS_API UFakeBulletComponent : public UActorComponent
{
	GENERATED_BODY()

public:

	UFakeBulletComponent();

#pragma region Initialization
public:

	FTransform InitialTransform;

	virtual void Initiate();

#pragma endregion

#pragma region Simulation
public:

	//Amount of iterations to perform in a execution, the higher the most detailed but more expensive
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Simulation")
	int32 ExecutionSubsteps = 3;

	//Amount of time simulated calculated. Keep in mind, this will still be a frame execution so this only affects at the math calculations aplied to the settings
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Simulation")
	float SimulatedTime = 1;

	/*
	* Executes the a simulation for each source that was give in.
	* @param Sources Array of transforms from where the execution start will be generated.
	* @param Offsets Indicates the offset applied to the sources based on the final endpoint
	*/
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	void ExecuteSimulation(UPARAM(ref) TArray<FTransform>& Sources, TArray<FVector>& Offsets);
	
	/*
	* Calculates the next step into the simulation of the bullet
	* @param DeltaTime the amount of time elapsed in execution
	* @param Offset Is the offset applied to the origin for multi steps simulations to correctly offset the trace
	* @param Origin Transform from where starts the simulation at the frame
	* @return Returns true if no hit would force the bullet to stop, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	virtual bool SimulationStep(const double DeltaTime, FVector& Offset, FTransform& Origin);
	bool SimulationStep(const double DeltaTime, FTransform& Origin);

	virtual void Deactivate() override;

protected:

	//Settings for the bullet behaviour. Keep in mind that normally you will spawn the bullet, so you should define the settings in the "Create Bullet Settings" of the weapon
	UPROPERTY(EditAnywhere, Category = "Simulation")
	FBulletSettings PhysicsSettings;

	//Pointer to the settings applied
	FBulletSettings* CS;

	//Ammount of frames to ignore all collisions (experimental)
	int32 SkippingSteps;

	//Current speed of the bullet
	double CurrentSpeed;

	//Strength to penetrate
	double PenetrationStrength;

	//Innertia of the gravity gathered
	FVector GravityIntertia = FVector(0);

	//List of actors to ignore in the traces
	TSet<AActor*> ActorsToIgnore;

	//Actors to ignore in the current frame
	TArray<TWeakObjectPtr<UPrimitiveComponent>> FrameComponentsToIgnore;

	//Number of times the bullet have bounced (limits the amount of bounces)
	int32 CurrentBounce = 0;

	UPROPERTY(BlueprintReadOnly)
	mutable FVector CurrentDirectionInterp;

#pragma endregion

#pragma region Collision Handle
public:

	/*
	* Checks the collision between the start and end position.
	* @param Start Position where the bullet is
	* @param End Position where the bullet should be
	* @param Impacts Array of the overlap impacts that suffered between those positions
	* @return true if there's some collision in between the start and end, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	virtual bool CollisionQuery(const FVector& Start, const FVector& End, TArray<FHitResult>& Impacts);

	/*
	* Handles the collision based in the type of surface and its collision and returns if has been stopped
	* @param Impacts The impacts to handle, it returns the impacts that have been processed (usually one)
	* @param QueryHits Array of actors that are impacted. Since one can't be hit showed twice and is based on the processed impacts so the max size will be the same Impact param output (or less)
	* @param ChangedTransform Indicates if it required to modify the transform on its own (reflected or penetrated)
	* @return true if the bullets can still travelling
	*/
	virtual bool SolvePhysicsHit(TArray<FHitResult>& Impacts, TArray<TWeakObjectPtr<UPrimitiveComponent>>& QueryHits, FTransform& CurrentTransform, bool& ChangedTransform);

	/**
	* Handles the hit interaction with the enviroment
	* @param Origin Is the origin from where the simulation starts, it may be modify to match the rotation
	* @param Offset Is the offset suffered by the origin, the desired offset will be added to it to take record of how moves through the world
	* @param DesiredOffset Is the offset reffering to the distance from the start to the end of the simulation step
	* @return Will be true if no obstacle stops the bullet
	*/
	virtual bool HandleHit(FTransform& Origin, FVector& Offset, const FVector& End, const FVector& DesiredOffset);

	/*
	* Modifies the trajectory of the bullet according to the impact reaction and hit
	* @param Hit Information of the hit to calculate the proper reaction
	* @param ImpactReaction Data of the reaction for the impacted surface
	* @param Transform Transform of the projectile when impacting. It will be modify if there's a modification in the trajectory
	* @return True if trajectory is modified and can still moving
	*/
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	bool ModifyTrajectory(const FHitResult& Hit, const FBulletImpactReaction& ImpactReaction, FTransform& Transform);

	//Traces by channel with the start, end, returning the hits and if there was a hit at all
	FORCEINLINE bool ChannelTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const;

	//Traces for object with the start, end, returning the hits and if there was a hit at all
	FORCEINLINE bool ObjectsTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const;

	//Traces by profile with the start, end, returning the hits and if there was a hit at all
	FORCEINLINE bool ProfileTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const;

#pragma endregion

#pragma region Math
public:

	/*
	* Calculates the next position that the bullet will have if there's no obstacles
	* @param DeltaTime the ammount of time elapsed for the bullet movement
	* @param EndPoint Position where the bullet will end
	* @param Offset Difference between the end and start position
	* @param OriginTransform Is the variable that tell where the actor starts from
	* @param Gravity Is the gravity that will be applied when calculated the displacement. Also will be added the frame gravity
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	virtual void CalculateTracePoints(const double DeltaTime, FVector& EndPoint, FVector& Offset, const FTransform& OriginTransform, FVector& Gravity) const;

	/*
	* Gets the direction of the bullet for this frame
	* @param DeltaTime Elapsed time in the simulation
	* @param OriginTransform Current transform used for smoothly interpolate direction
	* @return Vector direction of the bullet at current frame
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	FVector GetMovementDirection(const double& DeltaTime, const FTransform& OriginTransform) const;

	/*
	* Rotates the direction to try to face the target
	* @param DeltaTime Elapsed time in the simulation
	* @param OriginTransform Current transform used for smoothly interpolate direction
	* @return Vector direction of the bullet at current frame
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	FVector ReorientateToTarget(const double DeltaTime, const FTransform& From) const;

	/*
	* Calculates the expected new rotation and speed scale based on the hit and settings
	* @param Hit Information of the hit
	* @param Physics Information to determine the new rotation and speed
	* @param NewRotation Direction expected after the impact
	* @param SpeedScale Scale of current expected after the impact
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	void RicochetteRedirection(const FHitResult& Hit, const FBulletRicochet& Physics, FRotator& NewRotation, double& SpeedScale) const;
	
	/*
	* Calculates the remaining speed after an impact with the material setigns
	* @param OriginSpeed Initial speed that was been used
	* @param Material Physics material to base the math on
	* @return The new value for the speed
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	static double SpeedReescaleByMaterialRestitution(const double OriginSpeed, const UPhysicalMaterial* Material);

	/*
	* Indicates if can bullet can bounce with the physics
	* @param Physics Data to consider if can or not bounce
	* @return True if the bullet can bounce
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	//Returns if can bounce based on the physics information
	bool CanBounce(const FBulletRicochet& Physics) const;

	/*
	* Calculates the expected new rotation and speed scale based on the hit, deepness and settings
	* @param Hit Information of the hit
	* @param Physics Information to determine the new rotation, speed and deepness
	* @param NewRotation Direction expected after the impact
	* @param NewPosition Position expected after the impact
	* @param SpeedScale Scale of current expected after the impact
	* @param Deepness Distance scaled by the penetration strength for the physic surface impacted
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	void PenetrationRedirection(const FHitResult& Hit, const FBulletPenetration& Physics, FRotator& NewRotation, FVector& NewPosition, double& SpeedScale, double& Deepness) const;

	/*
	* Calculates a random vector direction based on impact
	* @param Hit Impact information to properly calculate the redirection
	* @param Physics Settings to affect the redirection vector
	* @return The direction of the physics
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	FVector GetRandomPenetrationDeviation(const FHitResult& Hit, const FBulletPenetration& Physics) const;

	/*
	* Indicates if can bullet can penetrate with the physics
	* @param Physics Data to consider if can or not penetrate
	* @return True if the bullet can penetrate
	*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Math")
	//Returns if can go through based on the physics information
	bool CanGoThrough(const FBulletPenetration& Physics) const;

protected:

	//Calculates if can bounce by chance
	bool RandomCanBounce(const FBulletRicochet& Physics) const;

#pragma endregion

#pragma region Events
public:

	//Gets called whenever the bullets receives a hit (does not matter if penetrates, bounces off or despawns)
	UPROPERTY(BlueprintAssignable)
	FBlockingHit OnHit;

	//Gets called whenever the bullets receives a hit that can not penetrate or bounce
	UPROPERTY(BlueprintAssignable)
	FBlockingHit OnBlockingHit;

#pragma endregion

#pragma region Getters
public:

	/*Gets current target's position. If position is a component, returns its position, otherwise returns the raw target positon*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Targeting")
	FVector GetTargetPosition() const;

	/*True if there's a valid target (component or non-zero target)*/
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation|Targeting")
	FORCEINLINE bool HasTarget() const { return CS->HomingComponent || !CS->TargetPostion.IsZero(); };

	//Returns what is using as physics settings
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE FBulletSettings GetPhysicsSettings() { return CS ? *CS : *(CS = &PhysicsSettings); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE int32 GetSkippingSubSteps() const { return SkippingSteps; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE double GetCurrentSpeed() const { return CurrentSpeed; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE double GetPenetrationStrength() const { return PenetrationStrength; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE FVector GetGravityIntertia() const { return GravityIntertia; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE TArray<AActor*> GetActorsToIgnore() const { return ActorsToIgnore.Array(); }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	void GetFrameComponentsToIgnore(TArray<UPrimitiveComponent*>& Array) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE int32 GetCurrentBounce () const { return CurrentBounce ; }


#pragma endregion

#pragma region Setters
public:

	/*Clears both targets*/
	UFUNCTION(BlueprintCallable, Category = "Advanced Combat System|Targeting")
	FORCEINLINE void ClearTarget() { CS->HomingComponent = nullptr;  CS->TargetPostion = FVector::Zero(); };

	/*Set targets*/
	UFUNCTION(BlueprintCallable, Category = "Advanced Combat System|Targeting")
	FORCEINLINE void SetTargetLocation(const FVector& Location) { CS->TargetPostion = FVector::Zero(); };

	/*Set targets*/
	UFUNCTION(BlueprintCallable, Category = "Advanced Combat System|Targeting")
	FORCEINLINE void SetTargetComponent(USceneComponent* Component) { CS->HomingComponent = TObjectPtr(*Component); };

	//Clears the lists of actors that will be ignored
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	FORCEINLINE void ClearIgnoreActors() { ActorsToIgnore.Empty(); };

	FORCEINLINE void AddActorsToIgnore(const TArray<AActor*>& Array) { ActorsToIgnore.Append(Array); };
	FORCEINLINE void AddActorsToIgnore(const TSet<AActor*>& Set) { ActorsToIgnore.Append(Set); };
	FORCEINLINE void AddActorsToIgnore(AActor* Actor) { ActorsToIgnore.Add(Actor); };

	FORCEINLINE void SetPhysicsSettings(FBulletSettings& NewSettings) { CS = &NewSettings; }
	FORCEINLINE void ClearPhysicsSettings() { CS = &PhysicsSettings; }

#pragma endregion

#pragma region Debug

public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Debug")
	TEnumAsByte<enum EDrawDebugTrace::Type> DrawDebugTrace;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Combat System|Debug")
	float DrawDebugTime = 5;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Combat System|Debug")
	FColor DrawDebugColor = FColor::Red;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Advanced Combat System|Debug")
	FColor DrawDebugHitColor = FColor::Green;

#pragma endregion

};
