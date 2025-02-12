// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#if WITH_EDITOR
#include "Logging/MessageLog.h" 
#endif

#include "CoreMinimal.h"
#include "ACSCoreFX.h"
#include "Components/ActorComponent.h"
#include "Engine/StreamableManager.h"
#include "NiagaraDataInterfaceExport.h"
#include "Chaos/ChaosEngineInterface.h"
#include "FXComponent.generated.h"

struct FImpactFX;
struct FHitResult;

struct FNiagaraFX;
struct FDecalFX;
struct FSoundFX;

class UDecalComponent;
class UNiagaraComponent;
class UAudioComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpawnFX, EPhysicalSurface, Surface, const UNiagaraComponent*, Niagara, const UDecalComponent*, Decal, const UAudioComponent*, Audio);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FUnhandled, EPhysicalSurface, Surface);

struct FNiagaraDecal
{

public:

	bool Simple;
	const FDecalFX* Decal;

	FNiagaraDecal(): Simple(true), Decal(nullptr) {};

	FNiagaraDecal(const FNiagaraDecal& Other): Simple(Other.Simple), Decal(Other.Decal){};

	FNiagaraDecal(const FDecalFX& FX, const bool simp) : Simple(simp), Decal(&FX){};

};

/**
 * Component that spawns FX on a impact based on the feeded data
 */
UCLASS(ClassGroup = (AdvancedCombatSystem), Blueprintable, meta = (BlueprintSpawnableComponent))
class ACS_EFFECTS_API UFXComponent : public UActorComponent, public INiagaraParticleCallbackHandler
{

	GENERATED_BODY()

#pragma region CORE
public:

	//Map of effects to generate per surface
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category="Special Effects")
	TMap<TEnumAsByte<EPhysicalSurface>, FImpactFX> SurfaceEffect;

protected:

	//Map that contains a pair of a Niagara System with it's decal settings
	TMap<const UNiagaraSystem*, const FNiagaraDecal> NiagaraDecal;

#pragma endregion

#pragma region DELEGATES
public:

	//Delegate called when spawns the complete effect on a handled surface
	UPROPERTY(BlueprintAssignable, Category = "Special Effects")
	FSpawnFX OnSpawnedFX;

	//Delegate called when spawns the complete effect on an unhandled surface
	UPROPERTY(BlueprintAssignable, Category = "Special Effects")
	FUnhandled OnUnhandledSurface;

#pragma endregion

#pragma region GENERATION
public:

	/*
	* Spawns all effects based on the impact
	* @return True if the surface is recogniced
	*/
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Generation")
	bool GenerateFX(const FHitResult& Impact, UNiagaraComponent*& NiagaraComp, UDecalComponent*& DecalComp, UAudioComponent*& AudioComp);

	/*
	* Generates all effects from the FX with the Impact data
	* @param FX Data structure to spawn the effects with
	*/
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Generation")
	void GenerateCompleteEffect(const FHitResult& Impact, const FImpactFX& FX, UNiagaraComponent*& NiagaraComp, UDecalComponent*& DecalComp, UAudioComponent*& AudioComp);

	UFUNCTION(BlueprintCallable, Category = "Special Effects|Generation")
	//Spawns a niagara effect with the impact information and the specified settings
	UNiagaraComponent* GenerateNiagaraEffect(const FHitResult& Impact, const FNiagaraFX& FX);

	UFUNCTION(BlueprintCallable, Category = "Special Effects|Generation")
	//Spawns a decal with the impact information and the specified settings
	UDecalComponent* GenerateDecalEffect(const FHitResult& Impact, const FDecalFX& FX, const double OverrideSize = -1);

	UFUNCTION(BlueprintCallable, Category = "Special Effects|Generation")
	//Spawns a sound with the impact information and the specified settings
	UAudioComponent* GenerateSoundEffect(const FHitResult& Impact, const FSoundFX& FX);

	//Called when a particle sends data to blueprints
	virtual void ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Datas, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset) override;

#pragma endregion

#pragma region Utils
protected:

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Special Effects")
	static FVector GetImpactReflection(const FHitResult& Impact);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Special Effects")
	static FRotator GetImpactSurfaceDirection(const FHitResult& Impact, bool bRandomRotation);

#pragma endregion

#pragma region SETTERS
public:

	//Overrides the parameters for the generation of decals on the surface of the hit
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Dynamics")
	void SetDecalParamsFromHit(const FHitResult& Hit, const FMaterialParameters& Parameters);

	//Appends parameters for the generation of decals on the surface of the hit
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Dynamics")
	void AddDecalParamsFromHit(const FHitResult& Hit, const FMaterialParameters& Parameters);

	//Overrides the parameters for the generation of decals on the surface
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Dynamics")
	void SetDecalParamsFromSurface(const TEnumAsByte<EPhysicalSurface> Surface, const FMaterialParameters& Parameters);

	//Appends parameters for the generation of decals on the surface
	UFUNCTION(BlueprintCallable, Category = "Special Effects|Dynamics")
	void AddDecalParamsFromSurface(const TEnumAsByte<EPhysicalSurface> Surface, const FMaterialParameters& Parameters);

#pragma endregion

};