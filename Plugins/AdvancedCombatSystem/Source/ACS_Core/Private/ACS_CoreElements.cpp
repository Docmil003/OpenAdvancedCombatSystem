// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#include "ACS_CoreElements.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

#if WITH_EDITOR

#define GET_NUM_OF_ACTIVE_ENUM(EnumClass) const UEnum* const Enum = StaticEnum<EnumClass>(); const FString HiddenMeta = TEXT("Hidden"); int n = 0; \
for (int32 EnumIndex = 1; EnumIndex < Enum->NumEnums(); ++EnumIndex) if (!Enum->HasMetaData(*HiddenMeta, EnumIndex)) ++n; \
return n;

int UCoreHelperFunctions::GetNumOfActiveEWeapon()
{
	GET_NUM_OF_ACTIVE_ENUM(EWeapon)
}

int UCoreHelperFunctions::GetNumOfActiveEDamage()
{
	GET_NUM_OF_ACTIVE_ENUM(EDamageType)
}

int UCoreHelperFunctions::GetNumOfActiveEAmmo()
{
	GET_NUM_OF_ACTIVE_ENUM(EAmmoType)
}

#endif

FDamageInfo UCoreHelperFunctions::BuildDamageInfo(const FHitResult& Hit, double Damage, AActor* Sender,
	TEnumAsByte<EDamageType> Type, bool Ambiental, float DeltaTime)
{
	return FDamageInfo
	(
		DeltaTime < 0 ? Damage : Damage * DeltaTime,
		Sender,
		Hit.ImpactPoint,
		Hit.Normal,
		UKismetMathLibrary::GetDirectionUnitVector(Hit.TraceStart, Hit.TraceEnd),
		Hit.Component.Get(),
		Hit.BoneName,
		Type,
		DeltaTime,
		Ambiental
	);
}

void UCoreHelperFunctions::GetClosestParentBoneInArray(const USkeletalMeshComponent* const Mesh, FName& Bone,
	const TArray<FName>& List)
{
#if WITH_EDITORONLY_DATA

	if(!Mesh)
	{
		//TODO
		UE_LOG(LogTemp, Error, TEXT("Trying to access to mesh to find the closest parent bone in array but Mesh is null"))
		return;
	}

	if (List.IsEmpty())
	{
		//TODO
		UE_LOG(LogTemp, Warning, TEXT("Trying to access to mesh to find the closest parent bone in array but List of bones is empty"))
			return;
	}

#endif


	while (!Bone.IsNone())
	{
		if (List.Contains(Bone)) return;

		Bone = Mesh->GetParentBone(Bone);

	}

}
