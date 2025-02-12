// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ACSCoreWeapons.h"
#include "Components/ActorComponent.h"
#include "ProjectileBehaviour.generated.h"

UCLASS(ClassGroup = (AdvancedCombatSystem), Blueprintable, meta = (BlueprintSpawnableComponent, ToolTip="A physic based bullet simulation that interacts with the enviroment with a hi-detailed and hi-customizable properties", ShortToolTip="A physic based bullet simulation"))
class ACS_WEAPONS_API  UProjectileBehaviour : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UProjectileBehaviour();

#pragma region Initiation/Deinitiation
public:	

	//Initializes the bullets settings for a proper simulation
	virtual void Initiate();

	//Activates the behaviour of the component
	virtual void Activate(bool bReset) override;

	//Deactivates the behaviour
	virtual void Deactivate() override;

protected:

	virtual void BeginPlay() override;
#pragma endregion

#pragma region Movement
public:

	/*
	* Calculates the next step into the simulation of the bullet. Deactivates the component if hit blocks the bullet
	* @param DeltaTime the amount of time elapsed since last execution
	*/
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	virtual void SimulationStep(const double DeltaTime);

protected:

	//Only when not using fixed tick
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
#pragma endregion

#pragma region Simulation

public:

	//Settings for the bullet behaviour. Keep in mind that normally you will spawn the bullet, so you should define the settings in the "Create Bullet Settings" of the weapon
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Simulation")
	FBulletSettings CurrentSettings;

	//Class that will be used as core of the movement
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, AdvancedDisplay, Category = "Simulation")
	TSubclassOf<class UFakeBulletComponent> CoreClass;

	//Continues or pause the simulation of the bullet
	void SetActivation(bool Activate);

	//Forces update to the project' setings simulation data
	UFUNCTION(BlueprintCallable, Category = "Simulation")
	static void SetGlobalSimulationData();

private:

	//Core that uses as physics engine
	class UFakeBulletComponent* Core;

	//Delta tick for fixed delta time
	static double InternalTick;

	//Indicates if uses a fixed tick rate
	static bool FixedTime;

	//Timer handle for fixed delta 
	FTimerHandle FixedTickHandle;

#pragma endregion

#pragma region GETTERS

public:

	//Gets the core component of the bullet
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	FORCEINLINE UFakeBulletComponent* GetCore() const { return Core; }

	//Indicates if the simulation runs on a fixed time
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	static bool IsFixedSimulation() { return FixedTime; };

	//Gets the latency (ms) of the simulation
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Simulation")
	static double GetSimulationLatency() { return FixedTime ? InternalTick / 1000.0f : -1; }

#pragma endregion

};
