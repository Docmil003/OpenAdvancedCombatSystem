// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "Components/ProjectileBehaviour.h"

#include "ACSCoreWeapons.h"
#include "Kismet/GameplayStatics.h"
#include "Components/FakeBulletComponent.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Math/UnrealMathUtility.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Chaos/ChaosEngineInterface.h"
#include "ACSSettings.h"

using namespace EDrawDebugTrace;
using namespace EFrictionCombineMode;

bool UProjectileBehaviour::FixedTime = 0;
double UProjectileBehaviour::InternalTick = 0;

UProjectileBehaviour::UProjectileBehaviour()
{

	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bAutoActivate = false;

	CoreClass = UFakeBulletComponent::StaticClass();

	Core = Cast<UFakeBulletComponent>(CreateDefaultSubobject("CoreElement", UFakeBulletComponent::StaticClass(), CoreClass, true, false));
	Core->ExecutionSubsteps = 1;
	Core->SetPhysicsSettings(CurrentSettings);

}

#pragma region Initiation/Deinitiation

void UProjectileBehaviour::Initiate()
{
	Core->Initiate();
	Core->InitialTransform = GetOwner()->GetTransform();
}

void UProjectileBehaviour::Activate(bool bReset)
{

	//This block is copy pasted from void UActorComponent::Activate(bool bReset) but removed the enable tick function
	if (bReset || ShouldActivate() == true)
	{
		SetActiveFlag(true);
		OnComponentActivated.Broadcast(this, bReset);
	}

	if (bReset) {
		//Reset settings
		Initiate();
		SetActivation(true);
	}
	
}

void UProjectileBehaviour::Deactivate()
{
	Super::Deactivate();

	SetActivation(false);
}

void UProjectileBehaviour::BeginPlay()
{
	Super::BeginPlay();

	//There should be a better place to set this. Probably by project settings but until then this will work just fine since is a cheap function
	SetGlobalSimulationData();

	//If fixed create a timer handle to control the execution
	if (FixedTime) {
		FTimerDelegate MovementDelegate;
		MovementDelegate.BindUObject(this, &UProjectileBehaviour::SimulationStep, UProjectileBehaviour::InternalTick);
		GetWorld()->GetTimerManager().SetTimer(FixedTickHandle, MovementDelegate, InternalTick, true, 0);
	}
	
	//If active initiate
	if (IsActive()) {
		Initiate();
		SetActivation(true);
	}
	else {
		SetActivation(false);
	}
	
}

#pragma endregion

#pragma region Movement

void UProjectileBehaviour::SimulationStep(const double DeltaTime)
{
	FTransform NewTransform = GetOwner()->GetActorTransform();

	if (!Core->SimulationStep(DeltaTime, NewTransform)) Deactivate();

	//The NewTransform contains also the direction rotation, so chooses between using all info or just displacement
	if (CurrentSettings.AlignActor)
		GetOwner()->SetActorTransform(NewTransform);
	else
		GetOwner()->SetActorLocation(NewTransform.GetLocation());
	
}

void UProjectileBehaviour::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SimulationStep(DeltaTime);
}

#pragma endregion

#pragma region Simulation

void UProjectileBehaviour::SetActivation(bool Activate)
{
	if (Activate) { //Unpause timer or Enable tick
		if (UProjectileBehaviour::FixedTime)
			GetOwner()->GetWorldTimerManager().UnPauseTimer(FixedTickHandle);
		else 
			SetComponentTickEnabled(true);
	}
	else { //Pause timer or Disable tick
		if (UProjectileBehaviour::FixedTime)
			GetOwner()->GetWorldTimerManager().PauseTimer(FixedTickHandle);
		else
			SetComponentTickEnabled(false);
	}
}

void UProjectileBehaviour::SetGlobalSimulationData()
{
	FixedTime = UACSSettings::Get()->bBulletFixedTick;
	InternalTick = FixedTime ? UACSSettings::Get()->BulletFixedTick : -1;
}

#pragma endregion