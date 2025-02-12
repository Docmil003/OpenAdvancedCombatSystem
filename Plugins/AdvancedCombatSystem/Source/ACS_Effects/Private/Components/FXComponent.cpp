// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "Components/FXComponent.h"
#include "ACSCoreFX.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"
#include "NiagaraComponent.h"
#include "Engine/DecalActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/DecalComponent.h"

DEFINE_LOG_CATEGORY(AdvancedCombatSystem_Effects_Log);

using namespace EAttachLocation;

#define APPEND_PARAMS(ParamName) Decal.MaterialParams.ParamName.Map.Append(Parameters.ParamName.Map);
#define SET_PARAMS(ParamName) Decal.MaterialParams.ParamName.Map = Parameters.ParamName.Map;

template <typename T>
inline T RandomArrayElem(const TArray<T>& Arr) {
	return Arr[FMath::RandRange(int32(0), Arr.Num() - 1)];
}

#pragma region GENERATION

bool UFXComponent::GenerateFX(const FHitResult& Impact, UNiagaraComponent*& NiagaraComp, UDecalComponent*& DecalComp, UAudioComponent*& AudioComp)
{
	EPhysicalSurface Surface = UGameplayStatics::GetSurfaceType(Impact);

	if (FImpactFX* FX = SurfaceEffect.Find(Surface)) //Handled surface
	{
		GenerateCompleteEffect(Impact, *FX, NiagaraComp, DecalComp, AudioComp);
		OnSpawnedFX.Broadcast(Surface, NiagaraComp, DecalComp, AudioComp);
		return true;
	}
	
	UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Has impacted a surface with no data (%i)"), UGameplayStatics::GetSurfaceType(Impact))
	//Unhandled surface
	OnUnhandledSurface.Broadcast(Surface);
	return false;
	
}

void UFXComponent::GenerateCompleteEffect(const FHitResult& Impact, const FImpactFX& FX, UNiagaraComponent*& NiagaraComp, UDecalComponent*& DecalComp, UAudioComponent*& AudioComp)
{
	if (FX.bGeneratesNiagara)	NiagaraComp	=	GenerateNiagaraEffect(Impact, FX.NiagaraFX);
	if (FX.bGeneratesDecal)		DecalComp	=	GenerateDecalEffect(Impact, FX.DecalFX);
	if (FX.bGeneratesSound)		AudioComp	=	GenerateSoundEffect(Impact, FX.SoundFX);
}

UNiagaraComponent* UFXComponent::GenerateNiagaraEffect(const FHitResult& Impact, const FNiagaraFX& FX)
{

	if (FX.SystemAssets.IsEmpty()) {
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Not found data for surface %i"), UGameplayStatics::GetSurfaceType(Impact));
		return nullptr;
	}

	UNiagaraComponent* Niagara;
	UNiagaraSystem* System = &*RandomArrayElem(FX.SystemAssets).Get();

	FRotator Rotation = (FX.bAlignToNormal ? FVector(Impact.Normal) : GetImpactReflection(Impact)).Rotation();

	if (FX.bAttached) 
	{
		Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
			System,							//Niagara System
			Impact.GetComponent(),			//Attach to
			Impact.BoneName,				//Bone
			Impact.ImpactPoint,				//Location
			Rotation,						//Rotation
			KeepWorldPosition,				//Attach location
			true,							//Autodestroy
			false,							//Autoactivate
			ENCPoolMethod::AutoRelease,		//Pooling
			true							//Precull
		);
	}
	else 
	{
		Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,							//World
			System,							//Niagara System
			Impact.ImpactPoint,				//Location
			Rotation,						//Rotation
			FVector(1),						//scale
			true,							//autodestroy
			false,							//autoactivate
			ENCPoolMethod::AutoRelease,		//Pooling
			true							//Precull
		);
	}

	if (FX.bGeneratesDecal)
	{
		Niagara->SetVariableObject(FX.HandlerPropertyName, this);

		if (!NiagaraDecal.Contains(System)) { //If the sistem is not managed, adds the info

			FNiagaraDecal NewFX = FNiagaraDecal(FX.NiagaraDecal, FX.bSimpleDecalSpawn);
			NiagaraDecal.Add(System, NewFX);

		}
	}

	Niagara->SetCastShadow(FX.bCastShadows);

	Niagara->Activate(true);
	return Niagara;

}

UDecalComponent* UFXComponent::GenerateDecalEffect(const FHitResult& Impact, const FDecalFX& FX, const double OverrideSize)
{
	//Early return when
	if (FX.Materials.IsEmpty() ||				//No material
		FX.bAttached &&							//Does not attach
		Impact.GetComponent() &&				//Hasn't a attachable component
		!Impact.GetComponent()->bReceivesDecals	//The component to attach don't support decals
		) {
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Could not spawn decal for surface %i"), UGameplayStatics::GetSurfaceType(Impact))
		return nullptr;
	}

	
	UMaterialInterface* DecalMaterial = RandomArrayElem(FX.Materials).Get();

	if (FX.bDynamic)
	{
		UMaterialInstanceDynamic* Dyn = UMaterialInstanceDynamic::Create(DecalMaterial, this);
		DecalMaterial = Dyn;

		for (const auto& Pair : FX.MaterialParams.ScalarParameters)
			Dyn->SetScalarParameterValue(Pair.Key, Pair.Value);
		
		for (const auto& Pair : FX.MaterialParams.ColorParameters)
			Dyn->SetVectorParameterValue(Pair.Key, Pair.Value);

		for (const auto& Pair : FX.MaterialParams.TexturesParameters)
			Dyn->SetTextureParameterValue(Pair.Key, Pair.Value);

		for (const auto& Pair : FX.MaterialParams.RuntimeVirtualTextureParameters)
			Dyn->SetRuntimeVirtualTextureParameterValue(Pair.Key, Pair.Value);

		for (const auto& Pair : FX.MaterialParams.SparseVolumeTexturesParameters)
			Dyn->SetSparseVolumeTextureParameterValue(Pair.Key, Pair.Value);


		//Pre-Implemented stretching
		if (FX.bStreched)
		{
			Dyn->SetScalarParameterValue(
				TEXT("CorrelationFactor"),
				FMath::Abs(UKismetMathLibrary::GetDirectionUnitVector(Impact.TraceStart, Impact.TraceEnd).Dot(Impact.Normal))
			);
		}
	}

	UDecalComponent* Decal;

	FVector DecalSize = (OverrideSize <= 0 ?
		FVector(FX.DecalDepthness, FX.DecalSize, FX.DecalSize) :
		FVector(FX.DecalDepthness, OverrideSize, OverrideSize)
	);

	if (FX.bAttached)
	{
		Decal = UGameplayStatics::SpawnDecalAttached(
			DecalMaterial,						//Material
			DecalSize,							//Size
			Impact.GetComponent(),				//Attach Component
			Impact.BoneName,					//Attach Bone
			Impact.ImpactPoint,					//Location
			GetImpactSurfaceDirection(Impact, FX.bRandomRotation),	//Rotation
			EAttachLocation::KeepWorldPosition	//LocationType
		);
	}
	else 
	{
		Decal = UGameplayStatics::SpawnDecalAtLocation(
			this,					//World
			DecalMaterial,			//Material
			DecalSize,				//Size
			Impact.ImpactPoint,		//Location
			GetImpactSurfaceDirection(Impact, FX.bRandomRotation)	//Rotation
		);
	}
	
	return Decal;

}

UAudioComponent* UFXComponent::GenerateSoundEffect(const FHitResult& Impact, const FSoundFX& FX)
{
	if (!FX.Sound) {
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Not found sound for effect on surface %i"), UGameplayStatics::GetSurfaceType(Impact));
		return nullptr;
	}

	const double& Volume = FX.bRandomVolume ? float(FX.VolumeRange) : FX.Volume;
	const double& Pitch = FX.bRandomPitch ? float(FX.PitchRange) : FX.Pitch;

	if (FX.bAttached) 
	{
		return UGameplayStatics::SpawnSoundAttached(
			FX.Sound,				//Sound
			Impact.GetComponent(),	//AttachToComponent
			Impact.BoneName,		//AttachPointName
			Impact.ImpactPoint,		//Location
			EAttachLocation::KeepWorldPosition,	//LocationType
			false,					//bStopWhenAttachedToDestroyed
			Volume,					//VolumeMultiplier
			Pitch,					//PitchMultiplier
			0,						//StartTime
			FX.Attenuation,			//AttenuationSettings
			FX.Concurrency,			//ConcurrencySettings
			true					//bAutoDestroy
		);
	}
	else
	{
		return UGameplayStatics::SpawnSoundAtLocation(
			this,
			FX.Sound,			//Sound
			Impact.ImpactPoint,	//Location
			UKismetMathLibrary::GetDirectionUnitVector(Impact.TraceStart, Impact.TraceEnd).Rotation(), //Rotation
			Volume,				//Volume
			Pitch,				//Pitch
			0,					//StartTime
			FX.Attenuation,		//Attenuation
			FX.Concurrency,		//Concurrency
			true				//Autodestroy
		);
	}
}

void UFXComponent::ReceiveParticleData_Implementation(const TArray<FBasicParticleData>& Datas, UNiagaraSystem* NiagaraSystem, const FVector& SimulationPositionOffset)
{
	if (const FNiagaraDecal* Info = NiagaraDecal.Find(NiagaraSystem))
	{
		TArray<FHitResult> HitResults;
		HitResults.Init(FHitResult(), Datas.Num());

		auto Hit = HitResults.CreateIterator();
		auto Data = Datas.CreateConstIterator();

		if (Info->Simple) {
			while (Hit && Data) 
			{
				Hit->ImpactPoint = Data->Position;
				Hit->ImpactNormal = Hit->Normal = Data->Velocity.GetSafeNormal();
				Hit->Distance = Data->Size;
				++Hit;
				++Data;
			}
		}
		else 
		{
			while (Hit && Data)
			{
				UKismetSystemLibrary::LineTraceSingle(
					this,
					Data->Position - Data->Velocity * GetWorld()->DeltaTimeSeconds,
					Data->Position + Data->Velocity * GetWorld()->DeltaTimeSeconds,
					ETraceTypeQuery::TraceTypeQuery1,
					false,
					TArray<AActor*>(),
					EDrawDebugTrace::None,
					*Hit,
					true
				);
				Hit->Distance = Data->Size;
				++Hit;
				++Data;
			}
		}

		for (auto& H : HitResults)
		{
			GenerateDecalEffect(H, *Info->Decal, H.Distance);
		}
	}
	else {
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Not found data for %s"), *NiagaraSystem->GetName())
	}
}

#pragma endregion

#pragma region Utils

FVector UFXComponent::GetImpactReflection(const FHitResult& Impact)
{
	return UKismetMathLibrary::GetReflectionVector(	//Reflection
		UKismetMathLibrary::GetDirectionUnitVector(Impact.TraceStart, Impact.TraceEnd),	//Direction
		Impact.Normal);								//Normal
}

FRotator UFXComponent::GetImpactSurfaceDirection(const FHitResult& Impact, bool bRandomRotation)
{
	return FRotationMatrix::MakeFromXZ(
		-Impact.Normal,	//Normal must face to the inside
		(bRandomRotation ?	//Z will be how it is rotate "in the local Z" axis
			UKismetMathLibrary::RandomUnitVector() :
			UKismetMathLibrary::GetDirectionUnitVector(Impact.TraceEnd, Impact.TraceStart).Cross(-Impact.Normal)
			)
	).Rotator();
}

#pragma endregion

#pragma region SETTERS

void UFXComponent::SetDecalParamsFromHit(const FHitResult& Hit, const FMaterialParameters& Parameters)
{
	SetDecalParamsFromSurface(UGameplayStatics::GetSurfaceType(Hit), Parameters);
}

void UFXComponent::AddDecalParamsFromHit(const FHitResult& Hit, const FMaterialParameters& Parameters)
{
	AddDecalParamsFromSurface(UGameplayStatics::GetSurfaceType(Hit), Parameters);
}

void UFXComponent::SetDecalParamsFromSurface(const TEnumAsByte<EPhysicalSurface> Surface, const FMaterialParameters& Parameters)
{
	if (FImpactFX* Impact = SurfaceEffect.Find(Surface))
	{
		FDecalFX& Decal = Impact->DecalFX;
		SET_PARAMS(ScalarParams);
		SET_PARAMS(ColParams);
		SET_PARAMS(TexParams);
		SET_PARAMS(RVTParams);
		SET_PARAMS(SVTParams);
	}
	else
	{
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Not found data for surface %i"), Surface)
	}

}

void UFXComponent::AddDecalParamsFromSurface(const TEnumAsByte<EPhysicalSurface> Surface,
	const FMaterialParameters& Parameters)
{
	if (FImpactFX* Impact = SurfaceEffect.Find(Surface))
	{
		FDecalFX& Decal = Impact->DecalFX;
		APPEND_PARAMS(ScalarParams);
		APPEND_PARAMS(ColParams);
		APPEND_PARAMS(TexParams);
		APPEND_PARAMS(RVTParams);
		APPEND_PARAMS(SVTParams);
	}
	else {
		UE_LOG(AdvancedCombatSystem_Effects_Log, Warning, TEXT("Not found data for surface %i"), Surface)
	}
}

#pragma endregion