//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Components/BeamComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "ACS_CoreElements.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"

bool UBeamComponent::FixedTime = 0;
float UBeamComponent::InternalTick = 0.03;

template <typename T>
T GetSum(const TArray<T>& Arr){
	T sum = Arr[0]*0;
	for(const T& i : Arr) sum +=i;
	return sum;
}

//Sets default values for this component's properties
UBeamComponent::UBeamComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

//Called when the game starts
void UBeamComponent::BeginPlay()
{
	Super::BeginPlay();
	
	//TODO UBeamComponent::FixedTime = UACSettings::Get()->bBeamFixedTick;
	//TODO UBeamComponent::InternalTick = UACSettings::Get()->BeamInternalTick;
	
	//Prepare fixed execution handle
	if(FixedTime){
		FTimerDelegate UpdateDelegate;
		UpdateDelegate.BindLambda([this](){
			Update(InternalTick);
		});
	
		GetWorld()->GetTimerManager().SetTimer(FixedTickHandle, UpdateDelegate, InternalTick, true);
	
		GetWorld()->GetTimerManager().PauseTimer(FixedTickHandle);
	}
	
}

//Called every Frame
void UBeamComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	Update(LastTick = DeltaTime);
}

void UBeamComponent::Update(float DeltaTime)
{
	//Array of points where hits the beam
	TArray<FVector> Points = TArray<FVector>({ GetComponentLocation() });
	TArray<FBeamHitInfo> Hits;
	
	//If beam can continue or is deactivating update the length
	if (UpdateBeam(FBeamData(TArray<int32>({ 0 }), GetComponentLocation(), GetForwardVector(),
		FVector::Zero(), CurrentLength), Points, Hits) || !BeamActive
	){
		CurrentLength += DeltaTime * (BeamActive ? IncrementPerSecond : -DecrementPerSecond);
		CurrentLength = FMath::Clamp<int32>(CurrentLength, 0, MaxBeamLength);
	}
	
	//When stopped, stop all tick logic to save performance
	if(CurrentLength == 0 && !BeamActive){
		Deactivate();
	}
	
	if(!Hits.IsEmpty()) OnHit.Broadcast(Hits);
	
	//Notify about all the beam important points
	OnUpdate.Broadcast(Points);
	
}

bool UBeamComponent::UpdateBeam(FBeamData Data, TArray<FVector>& Points, TArray<FBeamHitInfo>& HitInfo)
{
	TArray<FHitResult> Hits;
	FVector EndLoc = Data.Start + Data.Direction * Data.Length;
	
	UKismetSystemLibrary::SphereTraceMulti(
		this,
		Data.Start + Data.Normal * (SphereRadius + 1),
		EndLoc + Data.Normal * (SphereRadius + 1),
		SphereRadius,
		TraceChannel,
		bTraceComplex,
		ActorsToIgnore,
		DrawDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
		Hits,
		true,
		FLinearColor::Red,
		FLinearColor::Green,
		FixedTime ? InternalTick : GetWorld()->DeltaTimeSeconds
	);
	
	auto It = Hits.CreateConstIterator();
	
	if(!It) return true;
	
	do {
		
		const FHitResult& Hit = *It;
		bool bBounce = GetSum(Data.Index) < MaxBounces && ReflectionSurface.Contains(UGameplayStatics::GetSurfaceType(Hit));
		FBeamData NewData = FBeamData(
			Data.Index,
			Hit.ImpactPoint,
			UKismetMathLibrary::GetReflectionVector(Data.Direction, Hit.ImpactNormal),
			Hit.ImpactNormal,
			bBounceRestartLength ? Data.Length : Data.Length - Hit.Distance
		);
		
		HitInfo.Add(FBeamHitInfo(Hit, Hit.GetActor(), Hit.bBlockingHit, bBounce));
		
		if(Hit.bBlockingHit){
			if(bBounce){
				++NewData.Index.Last();
				return UpdateBeam(NewData, Points, HitInfo);
			}
			else{
				if(Data.Index.Num() == 1 && bBranchesLengthLimited)
				{
					CurrentLength = CurrentLength - Data.Length + Hit.Distance + SphereRadius + 1;
					return false;
				}
				return true;
			}
		}
	
		if(bBounce){
			NewData.Index.Add(bBranching ? 0 : MaxBounces);
			UpdateBeam(NewData, Points, HitInfo);
		}
	
	} while (++It);
	
	Points.Add(EndLoc);
	return true;
	
}

void UBeamComponent::UpdateActorsToIgnore()
{
	//This is the tmp arr to hold the actors and append them
	ActorsToIgnore.Empty();
	TArray<AActor*> Current;
	
	for(TSubclassOf<AActor> Actor : ClassesToIgnore){
		UGameplayStatics::GetAllActorsOfClass(this, Actor, Current);
		ActorsToIgnore.Append(Current);
	}
	
}

void UBeamComponent::Activate(bool bReset)
{
	//Prevents enabling the tick event included in the supper
	if(bReset || ShouldActivate() == true)
	{
		SetActiveFlag(true);
		OnComponentActivated.Broadcast(this, bReset);
	}
}

void UBeamComponent::Deactivate()
{
	Super::Deactivate();
	
	//Deactivates the timer or tick to save performance
	if(FixedTime){
		GetOwner()->GetWorldTimerManager().PauseTimer(FixedTickHandle);
	}else{
		SetComponentTickEnabled(false);
	}
}

void UBeamComponent::StartBeam(bool ForceStart, bool RestartIgnore)
{
	//If is not active can either activate or fail the beam start
	if(!IsActive()){
		if(ForceStart) Activate(true);
		else return;
	}
	
	//Updates the actors to ignore if told to
	if(RestartIgnore) UpdateActorsToIgnore();
	
	//Indicates that the beam is active and growing
	BeamActive = true;
	
	//Unpause timer or enable tick
	if(FixedTime){
		GetOwner()->GetWorldTimerManager().UnPauseTimer(FixedTickHandle);
	}else{
		SetComponentTickEnabled(true);
	}
	
}

void UBeamComponent::StopBeam()
{
	//Pretty straightforward
	BeamActive = false;
}