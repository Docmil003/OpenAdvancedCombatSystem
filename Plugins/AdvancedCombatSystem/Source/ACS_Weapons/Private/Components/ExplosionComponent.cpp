//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Components/ExplosionComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ACSCoreWeapons.h"
#include "DrawDebugHelpers.h"

#pragma region Utils

inline AActor* GetTopOwner(AActor* Other) {return Other->GetOwner() ? GetTopOwner(Other->GetOwner()) : Other; }

#pragma endregion

//Sets default values for this component's properties
UExplosionComponent::UExplosionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
	DamageType = DamageType1;
}

#pragma region Affecting

void UExplosionComponent::Explode_Implementation(TArray<FActorDamageInfo>& DamageCaused)
{
	//If is deactivated notify the fail and early return
	if(!IsActive()){
		UE_LOG(AdvancedCombatSystem_Weapons_Log, Warning, TEXT("Attempted to explode but it is not active"))
		OnExplosionFailed.Broadcast();
		return;
	}
	
	//Deactivates to prevent to many calls
	Deactivate();
	
	DamageArea(DamageCaused);
	OnExplosionSuccess.Broadcast();
	PostExplosion();
	
}

#pragma endregion

#pragma region Autodestruction

void UExplosionComponent::PostExplosion()
{
	if(bDestroyOwner) GetOwner()->Destroy();
	else if(bDestroySelf) DestroyComponent();
	else if (bReactiveAfterExplode)
	{
		auto Lambda = [this](){Activate();};
		if(ReactivationDelay > 0){
			FTimerHandle Unhandled;
			GetWorld()->GetTimerManager().SetTimer(Unhandled, Lambda, ReactivationDelay, false);
		}
		else{
			GetWorld()->GetTimerManager().SetTimerForNextTick(Lambda);
		}
	}
}

#pragma endregion

#pragma region Damage

FDamageInfo UExplosionComponent::BuildDamageInfo_Implementation(const AActor* Victim) const
{
	FVector Direction = UKismetMathLibrary::GetDirectionUnitVector(GetComponentLocation(), Victim->GetActorLocation());
	return FDamageInfo(
		CalculateDamage(Victim),	//Damage		Calculates the damage for the explosion victim
		GetTopOwner(GetOwner()),	//Sender		Sets the sender as the top owner of the component
		GetComponentLocation(),		//Point			Set damage point as explosion source
		FVector::Zero(),			//Normal		Since there's no proper hit, theres no normal
		Direction,					//Direction		Uses the direction from the source to the hit actor
		nullptr,					//HitComponent	Can't know which component specifically hitted
		FName(),					//Bone			Requires a hit component
		DamageType					//Type			Sets the damage type as variable
	);
}

double UExplosionComponent::CalculateDamage(const AActor* Other, double& Normalized) const
{
	//Distance from source to the victim
	const double Distance = FVector::Distance(Other->GetActorLocation(), GetComponentLocation());
	
	//If has a curve as damage us it
	if(UCurveFloat* Curve = DamageCurve.Get()){
		double MinR, MaxR;
		Curve->GetTimeRange(MinR, MaxR);
		
		Normalized = Curve->GetFloatValue(UKismetMathLibrary::MapRangeClamped(Distance, 0, SphereOverlapRadius, MinR, MaxR));
	}
	else{ 
		//Inverts the normalize value to obtain a normalized "near"
		Normalized = 1.0 - UKismetMathLibrary::MapRangeClamped(
			Distance, DamageRadiusBias.X, SphereOverlapRadius - DamageRadiusBias.Y, 0, 1
		);
	}
	
	return Normalized * MaxDamage;
	
}

double UExplosionComponent::CalculateDamage(const AActor* Other) const
{
	double foo;
	return CalculateDamage(Other, foo);
}

void UExplosionComponent::DamageArea(TArray<FActorDamageInfo>& DamageCaused)
{
	DrawDebugExplosion();
	TArray<AActor*> ActorsToIgnore;
	
	//For each class appends the actors of the class to ignore array
	for(TSubclassOf<AActor>& i : ClassesToIgnore){
		TArray<AActor*> OutActors;
		UGameplayStatics::GetAllActorsOfClass(this, i, OutActors);
		ActorsToIgnore.Append(OutActors);
	}
	
	//If should ignore the owner adds it to ignore array
	if(bIgnoreOwner){
		ActorsToIgnore.Add(GetOwner());
	}
	
	TArray<AActor*> ActorsReached;
	
	//Gets all overlapped actors with the specified data
	UKismetSystemLibrary::SphereOverlapActors(this, GetComponentLocation(),
		SphereOverlapRadius, ObjectTypes, SpecificClassToFilter, ActorsToIgnore, ActorsReached
	);
	
	DamageCaused.SetNum(ActorsReached.Num());
	
	auto It = DamageCaused.CreateIterator();
	for(auto& Victim : ActorsReached){
		*It++ = FActorDamageInfo(Victim, BuildDamageInfo(Victim));
	}
	
}

#pragma endregion

#pragma region Debug

void UExplosionComponent::DrawDebugExplosion()
{
	DrawDebugSphere(GetWorld(), GetComponentLocation(), SphereOverlapRadius, 32, FColor::Red, false, 5);
}

#pragma endregion