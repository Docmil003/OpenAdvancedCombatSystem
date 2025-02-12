// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.



#include "Components/MeleeComponent.h"

#include "Kismet/GameplayStatics.h"

#include "Kismet/KismetMathLibrary.h"

#include "Engine/World.h"

#include "Kismet/GameplayStatics.h"

#include "ACS_CoreElements.h"



UMeleeComponent::UMeleeComponent()

{

	PrimaryComponentTick.bCanEverTick = true;

	PrimaryComponentTick.bStartWithTickEnabled = false;

	SetComponentTickEnabled(false);

}



#pragma region Inherance



void UMeleeComponent::Activate(bool bReset)

{

	Super::Activate(bReset);



	//Restarts the actors to ignore and activates tick

	if(bReset) ResetActorsToIgnore(true);



	AActor* Owner = GetOwner();

	while (Owner)

	{

		ActorsToIgnore.AddUnique(Owner);

		Owner = Owner->GetOwner();

	}



	SetComponentTickEnabled(true);

}



void UMeleeComponent::Deactivate()

{

	Super::Deactivate();



	//Clear values and stops tick

	SetComponentTickEnabled(false);

	ActorsToIgnore.Empty();

	PreviousLoc = FVector::Zero();

	HitActors = 0;

}



void UMeleeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)

{

	//Each tick query for hit

	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HitQuery();

}



#pragma endregion



#pragma region Tracing



bool UMeleeComponent::TraceCast(TArray<FHitResult>& Results) const

{

	//Chooses the shape of trace

	switch (QueryShape)

	{

	case EQueryShape::Line: 

		switch (TraceMethod)	//Chooses the method of trace

		{

		case ETracingMethod::Channel: return UKismetSystemLibrary::LineTraceMulti(this, GetStart(), GetEnd(), TraceChannel, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Objects: return UKismetSystemLibrary::LineTraceMultiForObjects(this, GetStart(), GetEnd(), ObjectsTypes, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Profile: return UKismetSystemLibrary::LineTraceMultiByProfile(this, GetStart(), GetEnd(), ProfileName, false, ActorsToIgnore, DebugDraw, Results, true);

		}

	case EQueryShape::Sphere:

		switch (TraceMethod)	//Chooses the method of trace

		{

		case ETracingMethod::Channel: return UKismetSystemLibrary::SphereTraceMulti(this, GetStart(), GetEnd(), GetRadius(), TraceChannel, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Objects: return UKismetSystemLibrary::SphereTraceMultiForObjects(this, GetStart(), GetEnd(), GetRadius(), ObjectsTypes, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Profile: return UKismetSystemLibrary::SphereTraceMultiByProfile(this, GetStart(), GetEnd(), GetRadius(), ProfileName, false, ActorsToIgnore, DebugDraw, Results, true);

		}

	case EQueryShape::Box:

		switch (TraceMethod)	//Chooses the method of trace

		{

		case ETracingMethod::Channel: return UKismetSystemLibrary::BoxTraceMulti(this, GetStart(), GetEnd(), GetHalfSize(), GetOrientation(), TraceChannel, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Objects: return UKismetSystemLibrary::BoxTraceMultiForObjects(this, GetStart(), GetEnd(), GetHalfSize(), GetOrientation(), ObjectsTypes, false, ActorsToIgnore, DebugDraw, Results, true);

		case ETracingMethod::Profile: return UKismetSystemLibrary::BoxTraceMultiByProfile(this, GetStart(), GetEnd(), GetHalfSize(), GetOrientation(), ProfileName, false, ActorsToIgnore, DebugDraw, Results, true);

		}

	}

	return false;

}



#pragma endregion



#pragma region Utils



void UMeleeComponent::ResetActorsToIgnore(const bool bDeep)

{

	if (bDeep) {



		ActorsToIgnore.Empty();

		

		TArray<AActor*> OutActors;

		for (TSubclassOf<AActor> Class : ClassesToIgnore) {

			UGameplayStatics::GetAllActorsOfClass(this, Class, OutActors);

			ActorsToIgnore.Append(OutActors);

		}

	}

	else {



		auto It = ActorsToIgnore.CreateIterator();

		It.SetToEnd();



		//Hitted actors had been appended to the end.

		for (int32 i = 0; i < HitActors; ++i)

		{

			It.RemoveCurrent();

			--It;

		}



	}

	HitActors = 0;

}



void UMeleeComponent::HitQuery()

{



	//Gets the hit query results

	TArray<FHitResult> Results;

	TraceCast(Results);



	//Set previous location to current for next frame

	PreviousLoc = GetEnd();



	TSet<AActor*> HitAlready;



	//For each hit notifies on the delegate

	for (const FHitResult& Hit : Results) {



		AActor* HitActor = Hit.GetActor();



		if (!bCanHitSameMultipleTimesAtOneTrace && HitAlready.Contains(HitActor))

		{

			UE_LOG(AdvancedCombatSystem_Weapons_Log, Display, TEXT("The actor %s has been hit multiple times by the same trace but is not handled"), *HitActor->GetName())

			continue;

		}



		HitAlready.Add(HitActor);



		OnHit.Broadcast(Hit, HitActor, this);



		//If can hit only once adds it to the ignore array

		if (bHitOnce) {

			ActorsToIgnore.AddUnique(HitActor);

			++HitActors;//says that added one to the ignore list

		}



		if (BlockingSurfaces.Contains(UGameplayStatics::GetSurfaceType(Hit)))

		{

			OnBlocked.Broadcast(Hit, HitActor, this);

		}



	}



	if (bGenerateMultiHitEvent)

	{

		//Sorts by actor since HitsSameManyTimes requires to be sorted

		Results.Sort([](const FHitResult& A, const FHitResult& B) {

			return A.GetActor()->GetName() < B.GetActor()->GetName();

		});

		

		auto It = Results.CreateIterator();



		while (It)

		{

			TArray<FHitResult> ActorHits;



			if (HitsSameManyTimes(It, ActorHits))

				OnMultiHit.Broadcast(ActorHits, ActorHits[0].GetActor());

		}



	}



}



bool UMeleeComponent::HitsSameManyTimes(THitIterator& It, TArray<FHitResult>& Hits)

{

	const AActor* TargetActor = It->GetActor();



	Hits.Add(*It);

	It.RemoveCurrent();



	while (++It)

	{

		if (It->GetActor() == TargetActor)

		{

			Hits.Add(*It);

			It.RemoveCurrent();

		}

		else break;

	}



	return Hits.Num() > 1;

}



#pragma endregion



#pragma region Getters



FVector UMeleeComponent::GetStart() const

{

	//Gets previous location, if none, calculates it from the velocity and direction added to the end

	return PreviousLoc.IsZero() ?

		GetEnd() + GetSourceForward() * GetSourceFrameDistance() :

		GetFixedPreviousLocation();

}



float UMeleeComponent::GetSourceFrameDistance() const

{

	return GetWorld()->DeltaTimeSeconds * GetSourceConst()->GetComponentVelocity().Length();

}



FVector UMeleeComponent::GetFixedPreviousLocation() const

{

	return PreviousLoc.Equals(GetEnd(), MinDistance) ? PreviousLoc + FallBack : PreviousLoc;

}





#pragma endregion

