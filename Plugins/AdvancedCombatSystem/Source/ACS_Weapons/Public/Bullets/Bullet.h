//Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ACSBulletPoolingInterface.h"
#include "Bullet.generated.h"

class UProjectileBehaviour;

UCLASS()
class ACS_WEAPONS_API ABullet : public AActor, public IACSBulletPoolingInterface
{

	GENERATED_BODY()
	
public:

	//Sets default values for this actor's properties
	ABullet();
	
#pragma region Initialization

protected:

	//Called when the game stars or when spawned
	virtual void BeginPlay() override;
	
#pragma endregion

#pragma region Miscellaneous

protected:

	//Timer to autodeactivate
	FTimerHandle AutoDeactiveTimer;
	
	//Projectile Behaviour Component
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Components")
	TObjectPtr<UProjectileBehaviour> ProjectileComponent;
	
#pragma endregion

#pragma region Pool Control

public:

	//Amount of time can be active before autodeactivating
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool|Timelife")
	float DeactivationCountdown = 10;
	
	//Amount of time can be unactive before despawning. Only when not forced to persist
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Pool|Timelife")
	float InactiveLimit = 10;
	
	//Deactivates the bullet and sets itself to rest
	UFUNCTION(BlueprintCallable, Category = "Pool")
	bool RestInPool();
	
	#pragma region Interface
	
	/* Simulates the spawning of a bullet
	* @param Transform Where the actor will be spawned
	* @param Settings Information for the bullet to initialize with
	*/
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pool")
	bool ActivateBullet(const FTransform& Transform, const UBulletInitiationSettings* Settings);
	bool ActivateBullet_Implementation(const FTransform& Transform, const UBulletInitiationSettings* Settings);
	
	//Amount of time can be unactive before despawning. Only when not forced to persist
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, BlueprintPure, Category = "Pool|Lifetime")
	float GetInactiveLimit() const;
	
	//Simulates the despawn of the bullet
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Pool")
	void PutBulletToRest();
	
	#pragma endregion

#pragma endregion

#pragma region Owners manage

public:

	//Indicate if should ignore the owner of this actor
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "Simulation")
	uint8 bIgnoreOwner = true;
	
	//Indicate if should ignore the owner of this actor recursively
	UPROPERTY(BlueprintReadOnly, EditInstanceOnly, Category = "Simulation", meta = (EditCondition="bIgnoreOwner"))
	uint8 bIgnoreAllOwners = true;
	
private:

	void IgnoreOwner();

#pragma endregion

#pragma region Events

public:

	//Gets called whenever the bullet hits. No mater if penetrate, block or ricochette
	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void OnImpact(const FHitResult& Hit, const FVector& DesiredOffset, AActor* HitActor);
	
	//Gets called whenever the bullets receives a hit that stops the bullet movement.
	UFUNCTION(BlueprintNativeEvent, Category = "Events")
	void OnBlock(const FHitResult& Hit, const FVector& DesiredOffset, AActor* HitActor);

#pragma endregion

#pragma region GETTERS

public:

	//Returns projectile subobject
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Components")
	FORCEINLINE UProjectileBehaviour* GetBulletComponent() const { return ProjectileComponent; }

#pragma endregion

};