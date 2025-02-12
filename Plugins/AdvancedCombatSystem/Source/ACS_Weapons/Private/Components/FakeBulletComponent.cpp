// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Components/FakeBulletComponent.h"

#include "ACSCoreWeapons.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Math/UnrealMathUtility.h"
#include "Components/SceneComponent.h"
#include "Components/PrimitiveComponent.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Chaos/ChaosEngineInterface.h"

using namespace EDrawDebugTrace;
using namespace EFrictionCombineMode;

typedef UKismetMathLibrary UKML;

inline bool ContainsOnlyIgnoreComponents(const TArray<TWeakObjectPtr<UPrimitiveComponent>>& IgnoreComps, const TArray<FHitResult>& Impacts) {
	return !Impacts.IsEmpty() && !IgnoreComps.IsEmpty() && 
		//No one found in ignore list
		!Impacts.ContainsByPredicate([&IgnoreComps](const FHitResult& Hit) { 
			//If component is invalid or not contained exit
			return !Hit.Component.IsValid() || !IgnoreComps.Contains(Hit.Component); 
	});
}

// Sets default values for this component's properties
UFakeBulletComponent::UFakeBulletComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bAutoActivate = true;

	SkippingSteps = CurrentSpeed = PenetrationStrength = 0;
	DrawDebugTrace = EDrawDebugTrace::None;
	ClearPhysicsSettings();

}

#pragma region Initialization

void UFakeBulletComponent::Initiate()
{
	CurrentBounce = 0;
	GravityIntertia = FVector::Zero();
	CurrentSpeed = CS->InitialSpeed;
	PenetrationStrength = CS->Resistance;
	FrameComponentsToIgnore.Empty();
	CurrentDirectionInterp = InitialTransform.GetRotation().Vector();

	if (ActorsToIgnore.IsEmpty()){
		for (const TSubclassOf<AActor>& Subclass : CS->IgnoreClasses) {
			TArray<AActor*> Out;
			UGameplayStatics::GetAllActorsOfClass(this, Subclass, Out);
			ActorsToIgnore.Append(Out);
		}
	}
}

#pragma endregion

#pragma region Simulation

void UFakeBulletComponent::ExecuteSimulation(TArray<FTransform>& Sources, TArray<FVector>& Offsets)
{
	Initiate();
	Offsets.Init(FVector::Zero(), Sources.Num());

	for (auto Origin = Sources.CreateIterator(); Origin; ++Origin)	//Execution for each source
	{
		for (int32 exec = 0; exec < ExecutionSubsteps; ++exec)			//Substep 
		{
			//If can't move (was blocked) interrupt the subbsteping
			if(!SimulationStep(SimulatedTime / ExecutionSubsteps, Offsets[Origin.GetIndex()], *Origin)) break;
		}
	}

}

bool UFakeBulletComponent::SimulationStep(const double DeltaTime, FVector& Offset, FTransform& Origin)
{
	FVector End, DesiredOffset;
	FVector* GravityUsed;

	if (CS->bGatherGravity) GravityUsed = &GravityIntertia;
	else GravityUsed = new FVector(FVector::ZeroVector); //create temporal pointer

	//Current speed gets scaled by the air resistance
	CurrentSpeed *= 1.0 - FMath::Clamp(CS->AirResistance * DeltaTime, 0.f, 1.f); 
	
	CalculateTracePoints(DeltaTime, End, DesiredOffset, Origin, *GravityUsed);
	
	//If was temporal value, delete the pointer
	if (!CS->bGatherGravity) delete GravityUsed;

	Origin.SetRotation(DesiredOffset.ToOrientationQuat());

	//if not ignores frames accept query collision.
	if (!SkippingSteps)
	{
		return HandleHit(Origin, Offset, End, DesiredOffset);
	}
	else {
		//remove remainging frames skipping and continues
		--SkippingSteps;
		Offset += DesiredOffset;
	}

	return true;
}

bool UFakeBulletComponent::SimulationStep(const double DeltaTime, FTransform& Origin)
{
	FVector Offset = FVector::Zero();
	const bool result = SimulationStep(DeltaTime, Offset, Origin);
	Origin.AddToTranslation(Offset);
	return result;
}

void UFakeBulletComponent::Deactivate()
{
	Super::Deactivate();
	ClearIgnoreActors();
}

#pragma endregion

#pragma region Collision Handle

bool UFakeBulletComponent::CollisionQuery(const FVector& Start, const FVector& End, TArray<FHitResult>& Impacts)
{
	bool value = false;
	//Easy access to the different trace methods
	switch (CS->TraceMethod)
	{
	case ETracingMethod::Channel: value = ChannelTrace(Start, End, Impacts); break;
	case ETracingMethod::Objects: value = ObjectsTrace(Start, End, Impacts); break;
	case ETracingMethod::Profile: value = ProfileTrace(Start, End, Impacts); break;
	}

	//If has only collided with actors to temporary ignore, ignore all temporal and re-cast
	if (ContainsOnlyIgnoreComponents(FrameComponentsToIgnore, Impacts)) {
		for (const auto& Comp : FrameComponentsToIgnore) ActorsToIgnore.Add(Comp->GetOwner());
		value = CollisionQuery(Start, End, Impacts);
		for (const auto& Comp : FrameComponentsToIgnore) ActorsToIgnore.Remove(Comp->GetOwner());
	}

	return value;
}

bool UFakeBulletComponent::SolvePhysicsHit(TArray<FHitResult>& Impacts, TArray<TWeakObjectPtr<UPrimitiveComponent>>& QueryHits,
	FTransform& CurrentTransform, bool& ChangedTransform)
{
	//Since to continue not necesesary means that the transform has been changed we need to export this value
	ChangedTransform = false;

	const TArray<TWeakObjectPtr<UPrimitiveComponent>> InitialHitedActors = QueryHits;
	QueryHits.Empty();

	//Iterates for each hit received
	for (auto It = Impacts.CreateIterator(); It; ++It)
	{

		//Adds the impact as processed and the actor as collided
		QueryHits.Add(It->Component);

		//if was impacted in previous frame skip. Impact not relevant
		if (InitialHitedActors.Contains(It->Component)) {
			UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("The bullet had already impacted %s in the previous frame. Skipping"), *It->Component->GetName())
			It.RemoveCurrent();
			continue;
		}

		if (!It->bBlockingHit) continue;

		//If can iteract try, otherwise will block
		FBulletImpactReaction* ImpactReaction = CS->SurfaceReaction.Find(UGameplayStatics::GetSurfaceType(*It));
		if (!ImpactReaction) ImpactReaction = &CS->DefaultSurfaceReaction;

		if (ImpactReaction->ImpactSolver == EImpactResponse::Ignore) {
			return true;
		}
		ChangedTransform = ModifyTrajectory(*It, *ImpactReaction, CurrentTransform);
		InitialTransform = CurrentTransform;
		return ChangedTransform; //If is false here it means it was blocked

	}

	//if finish the loop means that no relevant collision was found
	return true;
}

bool UFakeBulletComponent::HandleHit(FTransform& Origin, FVector& Offset, const FVector& End, const FVector& DesiredOffset)
{
	TArray<FHitResult> Hits;
	bool bContinues = true;


	//Checks if there's any type of collision in the displacement
	if (CollisionQuery(Origin.GetLocation() + Offset, End, Hits)) {

		//Forces to update the position
		bool ApplyTransform;

		//Temporaly holds the elements to ignore in this frame
		auto HittedActors = FrameComponentsToIgnore;

		
		if (SolvePhysicsHit(Hits, HittedActors, Origin, ApplyTransform)) {	//Bullet can continue

			if (!ApplyTransform) {
				Offset += DesiredOffset;
			}
			else { //when transform modificated force update the owner
				GetOwner()->SetActorTransform(Origin);
				Offset = FVector::Zero();
			}

		}
		else {	//if blocked completely deactivate
			Deactivate();
			bContinues = false;
		}

		//Adds the impacted components to be ignore in next frame
		FrameComponentsToIgnore = HittedActors;

		for (const FHitResult& hit : Hits)
		{
			OnHit.Broadcast(hit, DesiredOffset, hit.GetActor());				//On impact

			if (!bContinues)
			{
				OnBlockingHit.Broadcast(hit, DesiredOffset, hit.GetActor());	//On blocked
			}
		}

	}

	else Offset += DesiredOffset; //if no collisiton just continue
	

	return bContinues;
}

bool UFakeBulletComponent::ModifyTrajectory(const FHitResult& Hit, const FBulletImpactReaction& ImpactReaction, FTransform& Transform)
{
	//New data to modify the transform if the bullet dont deactivates
	FRotator NewRot = Transform.Rotator();
	double SpeedScale = 1;
	FVector NewLoc = Hit.ImpactPoint;

	//Riccochete the bullet
	if (ImpactReaction.ImpactSolver == EImpactResponse::Ricochete && CanBounce(ImpactReaction.Ricochet)) {
		CurrentBounce++;
		RicochetteRedirection(Hit, ImpactReaction.Ricochet, NewRot, SpeedScale);
	}
	//Penetrate
	else if (ImpactReaction.ImpactSolver == EImpactResponse::Penetrate && CanGoThrough(ImpactReaction.Penetration)) {
		double Deepness = 0;
		PenetrationRedirection(Hit, ImpactReaction.Penetration, NewRot, NewLoc, SpeedScale, Deepness);
		PenetrationStrength -= Deepness;
	}
	else return false; //if no reaction to surface the surface, will deactivate
	

	//Updates the bullet information
	CurrentSpeed *= FMath::Max(SpeedScale, 0);
	Transform.SetComponents(NewRot.Quaternion(), NewLoc, Transform.GetScale3D());

	return true;
}

bool UFakeBulletComponent::ChannelTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const
{
	return (CS->SphereRadius > 0 ?
		UKismetSystemLibrary::SphereTraceMulti(this, Start, End, CS->SphereRadius, CS->TraceChannel, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
		: UKismetSystemLibrary::LineTraceMulti(this, Start, End, CS->TraceChannel, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
	);
}

bool UFakeBulletComponent::ObjectsTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const
{
	return (CS->SphereRadius > 0 ?
		UKismetSystemLibrary::SphereTraceMultiForObjects(this, Start, End, CS->SphereRadius, CS->ObjectsTypes, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
		: UKismetSystemLibrary::LineTraceMultiForObjects(this, Start, End, CS->ObjectsTypes, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
		);
}

bool UFakeBulletComponent::ProfileTrace(const FVector Start, const FVector End, TArray<FHitResult>& Hits) const
{
	return (CS->SphereRadius > 0 ?
		UKismetSystemLibrary::SphereTraceMultiByProfile(this, Start, End, CS->SphereRadius, CS->ProfileName, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
		: UKismetSystemLibrary::LineTraceMultiByProfile(this, Start, End, CS->ProfileName, CS->TraceComplex, ActorsToIgnore.Array(), DrawDebugTrace, Hits, true, DrawDebugColor, DrawDebugHitColor, DrawDebugTime)
		);
}

#pragma endregion

#pragma region Math

void UFakeBulletComponent::CalculateTracePoints(const double DeltaTime, FVector& EndPoint,
	FVector& Offset, const FTransform& OriginTransform, FVector& Gravity) const
{
	//Calculates the gravity at this frame
	Gravity += CS->Gravity * CS->Mass * DeltaTime;

	//Direction where the bullet will go
	FVector WorldDirection = GetMovementDirection(DeltaTime, OriginTransform);
	FVector FrameGravity = Gravity * DeltaTime;

	//Gets the final velocity by reducing from the speed the gravity, limiting speed to positive speed
	double FrameVelocity = FMath::Max((CurrentSpeed - FrameGravity.Length()) * DeltaTime, 0);

	Offset = WorldDirection * FrameVelocity + FrameGravity;

	EndPoint = Offset + OriginTransform.GetLocation(); 
}

FVector UFakeBulletComponent::GetMovementDirection(const double& DeltaTime, const FTransform& OriginTransform) const
{
	return HasTarget() ?
		ReorientateToTarget(DeltaTime, OriginTransform) :		//Aim to target
		InitialTransform.TransformVector(CS->LocalDirection);	//Go forward
}

FVector UFakeBulletComponent::ReorientateToTarget(const double DeltaTime, const FTransform& From) const
{
	if (CS->bHommingByRotation) {
		return FMath::QInterpTo( //Use quaternion interpolation to avoid Euler interpolation errors
			From.GetRotation(),
			UKML::GetDirectionUnitVector(From.GetLocation(), GetTargetPosition()).ToOrientationQuat(),
			DeltaTime,
			CS->HomingDirectionSpeed
		).Vector();
	}
	else {
		CurrentDirectionInterp = FMath::VInterpTo(
			CurrentDirectionInterp,
			UKML::GetDirectionUnitVector(From.GetLocation(), GetTargetPosition()),
			DeltaTime,
			CS->HomingDirectionSpeed
		);
		FVector ScaledDirection = CS->HomingDirectionSpeedScale * CurrentDirectionInterp;
		return (ScaledDirection.Length() > 1 ? CurrentDirectionInterp.GetUnsafeNormal() : ScaledDirection);
	}
}

void UFakeBulletComponent::RicochetteRedirection(const FHitResult& Hit, const FBulletRicochet& Physics,
	FRotator& NewRotation, double& SpeedScale) const
{
	FVector DirectionUnitVector = UKML::GetDirectionUnitVector(Hit.TraceStart, Hit.TraceEnd);

	FVector ReflectionVector = UKML::GetReflectionVector(DirectionUnitVector, Hit.ImpactNormal);

	FVector RandomVectorInCone = UKML::RandomUnitVectorInConeInDegrees(ReflectionVector, Physics.RandomDeflectedAngle);

	NewRotation = RandomVectorInCone.Rotation();
	SpeedScale = SpeedReescaleByMaterialRestitution((100 - Physics.SpeedDecrement) / 100, Hit.PhysMaterial.Get());
	
}

double UFakeBulletComponent::SpeedReescaleByMaterialRestitution(const double OriginSpeed, const UPhysicalMaterial* Material)
{
	if (Material) {
		switch (Material->RestitutionCombineMode)
		{
		case Average: return (OriginSpeed + Material->Restitution) / 2;

		case Min: return FMath::Min(OriginSpeed, Material->Restitution);

		case Multiply: return OriginSpeed * Material->Restitution;

		case Max: return FMath::Max(OriginSpeed, Material->Restitution);

		default: 
			UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("The speed reescale is not set to any handle method (%i)"), Material->RestitutionCombineMode)
			return OriginSpeed;
		}
	}
	else {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("There is no physic material on the impacted surface"))
		return OriginSpeed;
	}
}

bool UFakeBulletComponent::CanBounce(const FBulletRicochet& Physics) const
{
	return Physics.Ricochets && RandomCanBounce(Physics);
}

void UFakeBulletComponent::PenetrationRedirection(const FHitResult& Hit, const FBulletPenetration& Physics,
	FRotator& NewRotation, FVector& NewPosition, double& SpeedScale, double& Deepness) const
{
	//Comp to trace against
	UPrimitiveComponent* Comp = Hit.GetComponent();

	FHitResult CompHit;
	FCollisionQueryParams CollisionQueryParams = FCollisionQueryParams();
	CollisionQueryParams.bTraceComplex = true;

#if !UE_BUILD_SHIPPING

	CollisionQueryParams.bDebugQuery = true;
	CollisionQueryParams.TraceTag = TEXT("PenetrationHit");

#endif

	//Trace from oposite direction to check where will exit. Keep in mind this is a line trace, not a sphere trace, so not always be return a value
	if (Comp->LineTraceComponent(CompHit, Hit.Location + (NewRotation.Vector() * Comp->Bounds.SphereRadius), Hit.Location, CollisionQueryParams)) {

		NewPosition = CompHit.Location;

		//Calculates redirection random in a cone from movement direction
		NewRotation = GetRandomPenetrationDeviation(Hit, Physics).Rotation();
		if (FVector::DotProduct(NewRotation.Vector(), CompHit.ImpactNormal) < 0) {

			//Keep an eye
			NewRotation = FMath::GetReflectionVector(-NewRotation.Vector(), FVector::CrossProduct(FVector::CrossProduct(CompHit.Normal, NewRotation.Vector()), CompHit.Normal)).Rotation();
		}

		//Distance traveled scaled by "density"
		Deepness = (NewPosition - Hit.Location).Length() * Physics.Hardness;

		//Speed scale down %. Phys /100 to normalize and Deepness /100 to use meters, so 10000
		SpeedScale = 1.0 - (Physics.PenetrationSpeedDecrement * Deepness / 10000);
	} else {
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT(
			"The impact could not be calculated from the other side of the component. "
			"This happens when the trace hits a surface that is not directly in front of it, and hits because of the trace radius. "
			"Keep in mind that is an unreal limitation due to the lack of options for do a trace component"
		))
	}
}

FVector UFakeBulletComponent::GetRandomPenetrationDeviation(const FHitResult& Hit, const FBulletPenetration& Physics) const
{
	//TODO Ensure is not inside the plane normal
	return UKML::RandomUnitVectorInConeInDegrees(
		UKML::GetDirectionUnitVector(Hit.TraceStart, Hit.TraceEnd),
		Physics.RandomDeviation
	);
}

bool UFakeBulletComponent::CanGoThrough(const FBulletPenetration& Physics) const
{
	return (
		Physics.Hardness >= 0					&& 
		Physics.Hardness <= PenetrationStrength && 
		UKML::RandomBoolWithWeight(Physics.PenetrationProb)
	);
}

void UFakeBulletComponent::GetFrameComponentsToIgnore(TArray<UPrimitiveComponent*>& Array) const
{
	Array.Empty(FrameComponentsToIgnore.Num());
	for (auto& Comp : FrameComponentsToIgnore)
		Array.Emplace(Comp.Get());
}

bool UFakeBulletComponent::RandomCanBounce(const FBulletRicochet& Physics) const
{
	float DeflectionProbability = 0;

	if (Physics.bInfiniteRicochet) 
		DeflectionProbability = Physics.DeflectionProbability;
	else if (Physics.DeflectionProb.Num() > CurrentBounce) 
		DeflectionProbability = Physics.DeflectionProb[CurrentBounce];

	return UKML::RandomBoolWithWeight(DeflectionProbability / 100.0f);
}

#pragma endregion

FVector UFakeBulletComponent::GetTargetPosition() const
{
	return CS->HomingComponent ? CS->HomingComponent->GetComponentLocation() : CS->TargetPostion;
}
