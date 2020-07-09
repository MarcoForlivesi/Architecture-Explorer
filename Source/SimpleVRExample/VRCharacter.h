// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "VRCharacter.generated.h"

UCLASS()
class SIMPLEVREXAMPLE_API AVRCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AVRCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	bool FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation);
	void UpdateSpline(const TArray<FVector> &Path);
	void UpdateDestinationMarker();
	void UpdateBlinker();
	void DrawTeleportPath(const TArray<FVector> &Path);
	FVector2D GetBlinkerCenter() const;

	void MoveForward(float throttle);
	void MoveRight(float throttle);

	void GripLeft();
	void ReleaseLeft();
	void GripRight();
	void ReleaseRight();

	void BeginTeleport();
	void FinishTeleport();

	void StartFade(float FromAlpha, float ToAlpha);


	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;
	class USceneComponent* VRRoot;

	UPROPERTY(VisibleAnywhere)
	class USplineComponent* TeleportPath;

	UPROPERTY()
	class AHandController* LeftController;
	UPROPERTY()
	class AHandController* RightController;


	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* DestinationMarker;

	UPROPERTY()
	class UPostProcessComponent* PostProcessComponent;

	class UMaterialInstanceDynamic* BlinkerMaterialInstance;

	UPROPERTY(EditAnywhere)
	float TeleportProjectileRadius = 10.0f;
	UPROPERTY(EditAnywhere)
	float TeleportProjectileSpeed = 800;
	
	UPROPERTY(EditAnywhere)
	float TeleportSimulationTime = 2.0f;

	UPROPERTY()
	TArray<class USplineMeshComponent*> TeleportPathMeshPool;
	
	UPROPERTY(EditAnywhere)
	float TeleportFadeTime = 1.0f;

	UPROPERTY(EditAnywhere)
	FVector TeleportProjectExtend = FVector(100.0f, 100.0f, 100);

	UPROPERTY(EditAnywhere)
	class UMaterialInterface* BlinkerMaterialBase;

	UPROPERTY(EditAnywhere)
	class UCurveFloat* RadiusVsVelocity;

	UPROPERTY(EditDefaultsOnly)
	class UStaticMesh* TeleportArchMesh;

	UPROPERTY(EditDefaultsOnly)
	class UMaterialInterface* TeleportArchMaterial;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHandController> LeftHandControllerClass;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AHandController> RightHandControllerClass;
};
