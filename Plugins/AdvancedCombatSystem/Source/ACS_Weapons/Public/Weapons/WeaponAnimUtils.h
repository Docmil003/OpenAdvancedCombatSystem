// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "WeaponAnimUtils.generated.h"

/**
 * Calls the delegate that triggers the trigger logic start
 */
UCLASS()
class ACS_WEAPONS_API UTriggerWeapon : public UAnimNotify
{
	GENERATED_BODY()
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

};

/**
 * Sets the weapon as unnjammed
 */
UCLASS()
class ACS_WEAPONS_API UUnJamWeapon : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

/** 
 * Calls the weapon to realize the ammo exchange logic
 */
UCLASS()
class ACS_WEAPONS_API UReloadWeapon : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

/**
 * Sets's the melee component to forget about all ignored actors
 */
UCLASS()
class ACS_WEAPONS_API UResetHitMemoryMeleeWeapon : public UAnimNotify
{
	GENERATED_BODY()

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bDeepClear;
};

/**
 * ACS Anim notifies
 */
UCLASS()
class ACS_WEAPONS_API UMeleeState : public UAnimNotifyState
{
	GENERATED_BODY()

	virtual void NotifyBegin(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(class USkeletalMeshComponent* MeshComp, class UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
	void SetActivation(class USkeletalMeshComponent* MeshComp, bool Active);

	//If true, will stop at beginning and start at end
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bInvert;
	
	//Indicate if remove ignored actors when activates
	UPROPERTY(EditAnywhere, Category = "Settings")
	bool bClearIgnoreActors;

};