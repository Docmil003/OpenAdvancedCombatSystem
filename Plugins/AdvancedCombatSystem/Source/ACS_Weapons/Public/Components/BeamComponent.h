// Copyright 2024 Kingsley Shyne Mattis Sogorb. All Rights Reserved.



#pragma once



#include "CoreMinimal.h"

#include "Engine/HitResult.h"

#include "Chaos/ChaosEngineInterface.h"

#include "Components/SceneComponent.h"

#include "BeamComponent.generated.h"



//Data-struct for the generation of a beam

USTRUCT(BlueprintType)

struct FBeamData {



	GENERATED_BODY()



	//Index of the beam (last element). Other elements represent it's fathers

	UPROPERTY(BlueprintReadOnly, Category = "Beam")

	TArray<int32> Index = TArray<int32>();



	//Position from where to start the beamtrace

	UPROPERTY(BlueprintReadWrite, Category = "Beam")

	FVector Start = FVector::Zero();



	//Direction that the beam will follow

	UPROPERTY(BlueprintReadWrite, Category = "Beam")

	FVector Direction = FVector::Zero();



	//Normal of surface if has. Used to avoid collide multiple times with the same

	UPROPERTY(BlueprintReadWrite, Category = "Beam")

	FVector Normal = FVector::Zero();



	//Length of the beam

	UPROPERTY(BlueprintReadWrite, Category = "Beam")

	double Length = 0;



	FBeamData(const TArray<int32> InIndex, const FVector InStart, const FVector InDirection, const FVector InNormal, const double InLength) :

		Index(InIndex), Start(InStart), Direction(InDirection), Normal(InNormal), Length(InLength) {}



	FBeamData() {}



};



USTRUCT(BlueprintType)

struct FBeamHitInfo

{

	GENERATED_BODY()



	//Hit result of the beam

	UPROPERTY(BlueprintReadWrite, Category = "Beam|Hit")

	FHitResult Hit = FHitResult();



	//Actor hitted by the beam

	UPROPERTY(BlueprintReadWrite, Category = "Beam|Hit")

	AActor* HitActor = nullptr;



	//Indicates if is a blocking hit

	UPROPERTY(BlueprintReadWrite, Category = "Beam|Hit")

	bool Block = true;



	//Indicates if bounces on the surface

	UPROPERTY(BlueprintReadWrite, Category = "Beam|Hit")

	bool Bounce = false;



	FBeamHitInfo(){}

	



	FBeamHitInfo(

		FHitResult InHit,

		AActor* InHitActor,

		bool InBlock,

		bool InBounce

	) : Hit(InHit), HitActor(InHitActor), Block(InBlock), Bounce(InBounce) {}



};



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnHit, TArray<FBeamHitInfo>, TickHitInfo);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBeamUpdate, TArray<FVector>, Points);



//This component is experimental so lacks of documentation and testing, use with care

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )

class ACS_WEAPONS_API  UBeamComponent : public USceneComponent

{

	GENERATED_BODY()



public:	



	// Sets default values for this component's properties

	UBeamComponent();



protected:



	// Called when the game starts

	virtual void BeginPlay() override;



#pragma region TimeHandling



private:



	//Delta tick for fixed delta time

	static float InternalTick;



	//Indicates if uses a fixed tick rate

	static bool FixedTime;



public: 



	//Activates the behaviour of the component

	virtual void Activate(bool bReset) override;



	//Deactivates the behaviour

	virtual void Deactivate() override;



	// Called every frame if not fixed

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;





private:



	//Timer handle for fixed delta 

	FTimerHandle FixedTickHandle;



#pragma endregion



#pragma region Beam



public:



	/*
	* Takes care of all the beam component tick
	* @param DeltaTime Time elapsed since last frame
	*/

	virtual void Update(const float DeltaTime);

	

	/*
	* Updates the beam managing collisions, bounce and trespassing
	* @param Index Index of the current beam to handle the max iterations
	* @param Start Position from which to start the beam
	* @param Direction Direction to follow by the beam
	* @param Length Length that the beam will cast
	* @param Points Array containing the main impact points
	* @return true if the beam hasn't received any blocking hit
	*/

	virtual bool UpdateBeam(FBeamData Data, TArray<FVector>& Points, TArray<FBeamHitInfo>& HitInfo);



	/*
	* Starts (or tries) to start the beam
	* @param ForceStart Indicates if forces by activating the component, to start the beam
	* @param RestartIfnore Indicates if should recalculate the actors to ignore
	*/

	UFUNCTION(BlueprintCallable, Category = "Beam")

	virtual void StartBeam(bool ForceStart = true, bool RestartIgnore = false);



	/*
	* Stops the beam. The component will be deactivated when the beam stoped completly
	*/

	UFUNCTION(BlueprintCallable, Category = "Beam")

	virtual void StopBeam();



	//Max length that the beam can reach

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam", meta = (Units="cm"))

	double MaxBeamLength = 5000;



	//Velocity of length incrementation

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam", meta = (Units = "cm"))

	double IncrementPerSecond = 1000;



	//Velocity of length decrementation

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam", meta = (Units = "cm"))

	double DecrementPerSecond = 500;



	//Indicate if show the traces

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Debug")

	bool DrawDebug = false;





protected:



	//Indicates if should increment or decrement the length

	bool BeamActive = false;



#pragma endregion



#pragma region Capabilities



public:	



	//Max amount of bounces for the beam. Please, keep in mind that high number of bounces may slow down the game since for each bounce a sphere trace is casted

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour", meta = (UIMax = 10, UIMin = 1))

	int32 MaxBounces = 8;



	//Indicates if max length of the branch should be the current length of the main beam

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour", meta = (DisplayName = "Restart Length In Bounce"))

	bool bBounceRestartLength = false;



	//Indicates if max length of the branch should be the current length of the main beam

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour", meta = (DisplayName = "Limit Branch Length To Main"))

	bool bBranchesLengthLimited = true;



	//Experimental feature. Split the trace when hit reflection surface with overlap query, it will create two branches, one that behaves like a normal overlap, and another that behaves as a reflected beam, Use with care, since it may crash your game if using a high max bounces range.

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour", meta = (DisplayName = "Branching"))

	bool bBranching = false;



	//Radius of the sphere trace

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour")

	float SphereRadius = 5;



	//Channel to cast the trace

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour")

	TEnumAsByte<ETraceTypeQuery> TraceChannel;



	//Indicates if should query agains complex collision

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour")

	bool bTraceComplex;



	//Array of default classes to ignore

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour")

	TArray<TSubclassOf<AActor>> ClassesToIgnore;



	//List of actors to ignore

	UPROPERTY(BlueprintReadWrite, EditInstanceOnly, Category = "Beam|Behaviour")

	TArray<AActor*> ActorsToIgnore;



	//Array of surfaces to reflect

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Beam|Behaviour")

	TArray<TEnumAsByte<EPhysicalSurface>> ReflectionSurface;



	//Actual lengthof the beam

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Beam")

	double CurrentLength = 0;



#pragma endregion



#pragma region Miscellaneous

public:



	//Called when the beam gets updated on each execution. Gives the important beam positions

	UPROPERTY(BlueprintAssignable)

	FBeamUpdate OnUpdate;



	//Called for each hit

	UPROPERTY(BlueprintAssignable)

	FOnHit OnHit;



	//Clears the actors to ignore and set them to the actors of the classes to ignore

	UFUNCTION(BlueprintCallable, Category="Beam")

	void UpdateActorsToIgnore();



	//Gets the update rate

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Beam")

	FORCEINLINE float GetUpdateRate() const { return FixedTime ? InternalTick : LastTick; }



	/*
	* Sets the increment value based on the duration expected
	* @param Duration Time required to reach the max length from complety stop
	*/
	UFUNCTION(BlueprintCallable, Category = "Beam")

	FORCEINLINE void SetIncrementByDuration(const float Duration) { IncrementPerSecond = MaxBeamLength / Duration; }



	/*
	* Sets the decrement value based on the duration expected
	* @param Duration Time required to completly stop from the max length
	*/

	UFUNCTION(BlueprintCallable, Category = "Beam")

	FORCEINLINE void SetDecrementByDuration(const float Duration) { DecrementPerSecond = MaxBeamLength / Duration; }





private:



	float LastTick;



#pragma endregion



};