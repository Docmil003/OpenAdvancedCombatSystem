//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "VT/RuntimeVirtualTexture.h"
#include "SparseVolumeTexture/SparseVolumeTexture.h"
#include "Engine/Texture.h"
#include "Materials/MaterialInterface.h"
#include "ACSCoreFX.generated.h"

#define ACS_FX_LOG AdvancedCombatSystem_Effects_Log
DECLARE_LOG_CATEGORY_EXTERN(AdvancedCombatSystem_Effects_Log, Log, All);

class UTexture;
class USoundBase;
class USoundAttenuation;
class USoundConcurrency;
class UMaterialInterface;
class UNiagaraSystem;

#define ARRAY_STRUCT_RANDOM(StructName, Element) 	\
StructName(Element const& Elem) : Array({Elem}){}	\
StructName(const TArray<Element>& Arr) : Array(Arr){}	\
StructName() {} 	\
int32 operator+=(Element const& Other) {return Array.Add(Other);}	\
int32 operator-=(Element const& Other) {return Array.Remove(Other);}	\
StructName operator=(Element const& Other) {Array.Empty(1); Array.Add(Other); return *this;}	\
StructName operator=(StructName const& Other) {if(this!=&Other) {Array = Other.Array;} return *this;}	\
operator Element() const { if(!Array.IsEmpty()) return Array[FMath::RandRange(0, Array.Num() - 1)]; else return nullptr;}

#pragma region Random return structs

/*
* This struct's default get returns a random number
*/
USTRUCT(BlueprintType)
struct FRandomRangedFloat{
	
	GENERATED_BODY()
	
	//Represents the minimal value that the float can have
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Random Value")
	float MinValue;
	
	//Represents the minimal value that the float can have
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Random Value")
	float MaxValue;
	
	FRandomRangedFloat(const FVector2D& Range) : MinValue(Range.X), MaxValue(Range.Y){}
	FRandomRangedFloat(const float& Value) : MinValue(Value), MaxValue(Value){}
	FRandomRangedFloat(const float& Min, const float& Max) : MinValue(Min), MaxValue(Max){}
	FRandomRangedFloat() : FRandomRangedFloat(0,1){}
	
	FRandomRangedFloat operator=(FRandomRangedFloat const& Other){
		if(this != &Other){
			MinValue = Other.MinValue;
			MaxValue = Other.MaxValue;
		}
		return *this;
	}
	
	//Gets a random value from stream
	float GetValueFromStream(const FRandomStream& Stream) const {return Stream.FRandRange(MinValue, MaxValue);}
	
	operator FVector2D() const {return FVector2D(MinValue, MaxValue); }
	operator float() const {return FMath::FRandRange(MinValue, MaxValue);}
	
}; typedef FRandomRangedFloat FRanFloat;

/*
* This struct's default get returns a random color
*/
USTRUCT(BlueprintType)
struct FRandomRangedColor{
	
	GENERATED_BODY()
	
	//Represents the minimal value that the float can have
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Random Value")
	FLinearColor MinValue;
	
	//Represents the minimal value that the float can have
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Random Value")
	FLinearColor MaxValue;
	
	FRandomRangedColor(const TTuple<FLinearColor, FLinearColor>& Range) : MinValue(Range.Key), MaxValue(Range.Value){}
	FRandomRangedColor(const FLinearColor& Value) : MinValue(Value), MaxValue(Value){}
	FRandomRangedColor() : FRandomRangedColor(FLinearColor::Black){}
	
	FRandomRangedColor operator=(FRandomRangedColor const& Other){
		if(this != &Other){
			MinValue = Other.MinValue;
			MaxValue = Other.MaxValue;
		}
		return *this;
	}
	
	operator FLinearColor() const { return FLinearColor::LerpUsingHSV(MinValue, MaxValue, FMath::FRand());}
	
}; typedef FRandomRangedColor FRanCol;

/*
* This struct's default get returns a random Texture
*/
USTRUCT(BlueprintType)
struct FArrayOfTexture{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RandomValue")
	TArray<UTexture*> Array;
	
	ARRAY_STRUCT_RANDOM(FArrayOfTexture, UTexture*)
	
}; typedef FArrayOfTexture FArrTex;

/*
* This struct's default get returns a random Runtime Virtual Texture
*/
USTRUCT(BlueprintType)
struct FArrayOfRuntimeVirtualTexture{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RandomValue")
	TArray<URuntimeVirtualTexture*> Array;
	
	ARRAY_STRUCT_RANDOM(FArrayOfRuntimeVirtualTexture, URuntimeVirtualTexture*)
	
}; typedef FArrayOfRuntimeVirtualTexture FArrRVT;

/*
* This struct's default get returns a random Sparse Volume Texture
*/
USTRUCT(BlueprintType)
struct FArrayOfSparseVolumeTexture{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "RandomValue")
	TArray<USparseVolumeTexture*> Array;
	
	ARRAY_STRUCT_RANDOM(FArrayOfSparseVolumeTexture, USparseVolumeTexture*)
	
}; typedef FArrayOfSparseVolumeTexture FArrSVT;

#pragma endregion

//Data-struct of sound effects to play
USTRUCT(BlueprintType)
struct FSoundFX
{
	
	GENERATED_BODY()
	
	//Main sound base to play
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	TObjectPtr<USoundBase> Sound;
	
	//Indicates if the sound should or not be attached
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	bool bAttached = false;

	//Indicates if the volume is randomized or fixed
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	bool bRandomVolume = false;
	
	//Fixed volume of the sound
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (DisplayName = "Volume Multiplier", EditCondition = "!bRandomVolume", EditConditionHides))
	double Volume = 1;
	
	//Scale range of the sound volume
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (EditCondition = "bRandomVolume", EditConditionHides))
	FRandomRangedFloat VolumeRange{ 1.5, 0.5 };
	
	//Indicates if uses a random or fixed pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio")
	bool bRandomPitch = false;
	
	//Value of the fixed pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (DisplayName = "Pitch Multiplier", EditCondition = "!bRandomPitch", EditConditionHides))
	double Pitch = 1;
	
	//Scale range of the sound pitch
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (EditCondition = "bRandomPitch", EditConditionHides))
	FRandomRangedFloat PitchRange{ 1.5, 0.5 };
	
	//Attenuation settigs reference to use
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (DisplayName = "Attenuation Settings"))
	TObjectPtr<USoundAttenuation> Attenuation;
	
	//Class concurrency to use for the sound
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Audio", meta = (DisplayName = "Concurrency Settings"))
	TObjectPtr<USoundConcurrency> Concurrency;
	
};

//Allows a easier implementation to access map data
template <typename T>
struct FNameValues
{
	TMap<FName, T>& Map;
	
	FNameValues(TMap<FName, T>& Other) : Map(Other){};
	
	operator TMap<FName, T>&() const {return Map;}
	
	FNameValues<T> operator=(const FNameValues<T>& Other){
		if(&Map != &Other.Map){
			Map = Other.Map;
		}
		return *this;
	}
	
};

//Struct that holds parameters to add to a material instance object
USTRUCT(BlueprintType)
struct FMaterialParameters
{
	
	GENERATED_BODY()
	
	//Array of pair (map), name-value (parameter's name-value). Only when using bDynamic
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	TMap<FName, FRandomRangedFloat> ScalarParameters;
	FNameValues<FRanFloat> ScalarParams{ScalarParameters};
	
	//Array of pair (map), name-value (parameter's name-value). Only when using bDynamic
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	TMap<FName, FRandomRangedColor> ColorParameters;
	FNameValues<FRanCol> ColParams{ColorParameters};
	
	//Array of pair (map), name-value (parameter's name-value). Only when using bDynamic
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	TMap<FName, FArrayOfTexture> TexturesParameters;
	FNameValues<FArrTex> TexParams{TexturesParameters};
	
	//Array of pair (map), name-value (parameter's name-value). Only when using bDynamic
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	TMap<FName, FArrayOfRuntimeVirtualTexture> RuntimeVirtualTextureParameters;
	FNameValues<FArrRVT> RVTParams{RuntimeVirtualTextureParameters};
	
	//Array of pair (map), name-value (parameter's name-value). Only when using bDynamic
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	TMap<FName, FArrayOfSparseVolumeTexture> SparseVolumeTexturesParameters;
	FNameValues<FArrSVT> SVTParams{SparseVolumeTexturesParameters};
	
	//Automatically selects whichever Map of values requires
	
	operator FNameValues<FRanFloat>() const {return ScalarParams;}
	operator FNameValues<FRanCol>() const {return ColParams;}
	operator FNameValues<FArrTex>() const {return TexParams;}
	operator FNameValues<FArrRVT>() const {return RVTParams;}
	operator FNameValues<FArrSVT>() const {return SVTParams;}
};

//Data-struct of decals info to spawn
USTRUCT(BlueprintType)
struct FDecalFX
{
	
	GENERATED_BODY()
	
	//Indicates if use the hard reference to materials (which increases the memory on spawn). Keep in mind that this will be added to the soft materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal")
	bool bMaterialsHardReference = true;
	
	//Array of materials
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal", meta = (DisplayName = "Materials List", EditCondition="bMaterialsHardReference"))
	TArray<TObjectPtr<UMaterialInterface>> Materials;
	
	//Array of materials (soft reference)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal", meta = (DisplayName = "Materials List Soft", EditCondition="!bMaterialsHardReference"))
	TArray<TSoftObjectPtr<UMaterialInterface>> SoftMaterials;
	
	//Indicates if the decal should be attached to the impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal")
	bool bAttached = true;
	
	//Indicates the size of the decal in YZ axes
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal")
	FRandomRangedFloat DecalSize = 50;
	
	//Scale of the decal in X axes (Depth)
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal")
	FRandomRangedFloat DecalDepthness = 20;
	
	//Indicates whether to randomly rotate the decal based on the hit's surface's normal plane. Should be false if using the built-in stretch
	bool bRandomRotation = true;
	
	//Indicates if should create a dynamic instance material for the decal. Must be true if want to stretch or use custom parameters.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic")
	bool bDynamic = false;
	
	//Indicates if should use the built-in decal stretching method to stretch the decal acording to the impact angle
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic", meta = (EditCondition = "bDynamic", EditConditionHides))
	bool bStreched = false;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal|Dynamic", meta = (EditCondition = "bDynamic", EditConditionHides))
	FMaterialParameters MaterialParams;
	
	FDecalFX(){
		if(!bMaterialsHardReference) return;
		
		for(const auto& Mat : Materials){
			SoftMaterials.AddUnique(Mat);
		}
		
		Materials.Empty();
		
	}
	
};

//Data-struct of niagara system to spawn
USTRUCT(BlueprintType)
struct FNiagaraFX
{
	
	GENERATED_BODY()
	
	//Array of possible systems to spawn
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara", meta = (DisplayName = "System Assets List"))
	TArray<TObjectPtr<UNiagaraSystem>> SystemAssets;
	
	//Indicates if the niagara should be attached to the surface hit actor
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara")
	uint8 bAttached:1 = false;
	
	//Indicates if the niagara should be orientated to the hit's surface or simulate the reflection based on direction and angle
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara")
	uint8 bAlignToNormal:1 = false;

	//Indicates if the niagara cast shadows
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara")
	uint8 bCastShadows:1 = false;
	
	//Name of the niagara variable that will hold the object to send the data to.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara|Event")
	FName HandlerPropertyName = FName("EventHandler");
	
	//Indicates if the niagara can generate decals on the impact. USE WITH CAUTION. Keep in mind that one decal may spawn for EACH event triggered which is likely to lead to framerate drops
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara|Decal")
	bool bGeneratesDecal = false;
	
	//Settings for the decals to spawn. Is recomended to avoid using dynamics. Attachment will not work if using simple decal spawn
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara|Decal", meta = (EditCondition="bGeneratesDecal", EditConditionHides))
	FDecalFX NiagaraDecal = FDecalFX();
	
	//Indicates if generates the decal based on the niagara information (An aptroximation) rather than a linetraced that is slower but more accurate
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara|Decal", meta = (EditCondition = "bGeneratesDecal", EditConditionHides))
	bool bSimpleDecalSpawn = true;
};

//Data-struct of a complete impact effect
USTRUCT(BlueprintType)
struct FImpactFX
{
	
	GENERATED_BODY()
	
	//Indicates if this impact should or not generate a Niagara particle effect
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara", meta = (InlineEditConditionToggle))
	bool bGeneratesNiagara = false;
	
	//Niagara Particle Effects data to generate on impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Niagara", meta = (EditCondition = "bGeneratesNiagara"))
	FNiagaraFX NiagaraFX;
	
	//Indicates if this impact should or not generate a Decal
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal", meta = (InlineEditConditionToggle))
	bool bGeneratesDecal = false;
	
	//Decal data to generate on impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Decal", meta = (EditCondition = "bGeneratesDecal"))
	FDecalFX DecalFX;
	
	//Indicates if this impact should or not generate a Sound effect
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound", meta = (InlineEditConditionToggle))
	bool bGeneratesSound = false;
	
	//Sound Effects data to generate on impact
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Sound", meta = (EditCondition = "bGeneratesSound"))
	FSoundFX SoundFX;
	
};