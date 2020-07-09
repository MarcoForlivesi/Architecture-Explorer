// Fill out your copyright notice in the Description page of Project Settings.


#include "VRCharacter.h"
#include "Camera\CameraComponent.h"
#include "MotionControllerComponent.h"
#include "HandController.h"

#include "Components\SceneComponent.h"
#include "Components\ChildActorComponent.h"
#include "Components\CapsuleComponent.h"
#include "Components\StaticMeshComponent.h"
#include "Components\SplineMeshComponent.h"
#include "Components\MeshComponent.h"
#include "Components\SkeletalMeshComponent.h"
#include "Components\PostProcessComponent.h"
#include "Materials\MaterialInstanceDynamic.h"
#include "Components\SplineComponent.h"

#include "HeadMountedDisplayFunctionLibrary.h"
#include "NavigationSystem.h"
#include "Kismet/GameplayStatics.h"

#include "Engine/StaticMesh.h"

// Sets default values
AVRCharacter::AVRCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	VRRoot = CreateDefaultSubobject<USceneComponent>(TEXT("VRRoot"));
	VRRoot->SetupAttachment(GetRootComponent());
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(VRRoot);

	TeleportPath = CreateDefaultSubobject<USplineComponent>(TEXT("TeleportPath"));
	TeleportPath->SetupAttachment(VRRoot);

	DestinationMarker = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DestinationMarker"));
	DestinationMarker->SetupAttachment(GetRootComponent());

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(GetRootComponent());
}

// Called when the game starts or when spawned
void AVRCharacter::BeginPlay()
{
	Super::BeginPlay();

	DestinationMarker->SetVisibility(false);
	UHeadMountedDisplayFunctionLibrary::SetTrackingOrigin(EHMDTrackingOrigin::Floor);

	if (BlinkerMaterialBase != nullptr)
	{
		BlinkerMaterialInstance = UMaterialInstanceDynamic::Create(BlinkerMaterialBase, this);
		PostProcessComponent->AddOrUpdateBlendable(BlinkerMaterialInstance);
	}

	LeftController = GetWorld()->SpawnActor<AHandController>(LeftHandControllerClass);
	if (LeftController != nullptr)
	{
		LeftController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		LeftController->SetHand(EControllerHand::Left);
		LeftController->SetOwner(this);
	}

	RightController = GetWorld()->SpawnActor<AHandController>(RightHandControllerClass);
	if (RightController != nullptr)
	{
		RightController->AttachToComponent(VRRoot, FAttachmentTransformRules::KeepRelativeTransform);
		RightController->SetHand(EControllerHand::Right);
		RightController->SetOwner(this);
	}

	LeftController->PairController(RightController);
	RightController->PairController(LeftController);
}

// Called every frame
void AVRCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector NewCameraOffset = Camera->GetComponentLocation() - GetActorLocation();
	NewCameraOffset.Z = 0;
	AddActorWorldOffset(NewCameraOffset);

	VRRoot->AddWorldOffset(-NewCameraOffset);

	UpdateDestinationMarker();
	UpdateBlinker();
}

bool AVRCharacter::FindTeleportDestination(TArray<FVector> &OutPath, FVector &OutLocation)
{
	FVector Start = RightController->GetActorLocation();
	FVector Look = RightController->GetActorForwardVector();

	FPredictProjectilePathParams PredictParams(TeleportProjectileRadius,
		Start,
		Look * TeleportProjectileSpeed,
		TeleportSimulationTime,
		ECollisionChannel::ECC_Visibility,
		this);



	PredictParams.bTraceComplex = true;

	FPredictProjectilePathResult PathResult;

	bool bHit = UGameplayStatics::PredictProjectilePath(this, PredictParams, PathResult);

	if (!bHit)
	{
		return false;
	}

	FNavLocation NavLocation;
	bool bOnNavMesh = UNavigationSystemV1::GetCurrent(GetWorld())->ProjectPointToNavigation(PathResult.HitResult.Location, NavLocation, TeleportProjectExtend);

	if (!bOnNavMesh)
	{
		return false;
	}

	for (FPredictProjectilePathPointData PointData : PathResult.PathData)
	{
		OutPath.Add(PointData.Location);
	}

	OutLocation = NavLocation.Location;

	return true;
}

void AVRCharacter::UpdateSpline(const TArray<FVector>& Path)
{
	TeleportPath->ClearSplinePoints(false);

	for (int32 i = 0; i < Path.Num(); ++i)
	{
		FVector LocalPosition = TeleportPath->GetComponentTransform().InverseTransformPosition(Path[i]);
		FSplinePoint SplinePoint(i, LocalPosition, ESplinePointType::Curve);

		TeleportPath->AddPoint(SplinePoint, false);
	}

	TeleportPath->UpdateSpline();
}


void AVRCharacter::UpdateDestinationMarker()
{
	TArray<FVector> Path;
	FVector Location;
	bool bHasDestination = FindTeleportDestination(Path, Location);

	if (bHasDestination)
	{
		DestinationMarker->SetWorldLocation(Location);
		DrawTeleportPath(Path);
	}
	else
	{
		DrawTeleportPath(TArray<FVector>());
	}

	DestinationMarker->SetVisibility(bHasDestination);
}

void AVRCharacter::UpdateBlinker()
{
	if (RadiusVsVelocity == nullptr || BlinkerMaterialInstance == nullptr)
	{
		return;
	}

	float speed = GetVelocity().Size();

	float radius = RadiusVsVelocity->GetFloatValue(speed);

	BlinkerMaterialInstance->SetScalarParameterValue(TEXT("Radius"), radius);

	FVector2D Center = GetBlinkerCenter();
	BlinkerMaterialInstance->SetVectorParameterValue(TEXT("Center"), FLinearColor(Center.X, Center.Y, 0.0f));
}

void AVRCharacter::DrawTeleportPath(const TArray<FVector> &Path)
{
	UpdateSpline(Path);

	for (USplineMeshComponent* SplineMesh : TeleportPathMeshPool)
	{
		SplineMesh->SetVisibility(false);
	}

	int32 SegmentNum = Path.Num() - 1;
	for (int32 i = 0; i < SegmentNum; ++i)
	{
		if (TeleportPathMeshPool.Num() <= i)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(TeleportPath, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStaticMesh(TeleportArchMesh);
			SplineMesh->SetMaterial(0, TeleportArchMaterial);
			SplineMesh->RegisterComponent();
			TeleportPathMeshPool.Add(SplineMesh);
		}

		USplineMeshComponent* SplineMesh = TeleportPathMeshPool[i];
		FVector StartPos, StartTangent, EndPosition, EndTangent;
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i, StartPos, StartTangent);
		TeleportPath->GetLocalLocationAndTangentAtSplinePoint(i + 1, EndPosition, EndTangent);
		SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPosition, EndTangent);
		SplineMesh->SetVisibility(true);
	}
}

FVector2D AVRCharacter::GetBlinkerCenter() const
{
	FVector MovementDirection = GetVelocity().GetSafeNormal();

	if (MovementDirection.IsNearlyZero())
	{
		return FVector2D(0.5f, 0.5f);
	}

	float sign = FGenericPlatformMath::Sign(FVector::DotProduct(Camera->GetForwardVector(), MovementDirection));
	FVector WorldStationaryLocation = Camera->GetComponentLocation() + sign * MovementDirection * 1000.0f;

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC == nullptr)
	{
		return FVector2D(0.5f, 0.5f);
	}


	FVector2D ScreenStationaryLocation;
	PC->ProjectWorldLocationToScreen(WorldStationaryLocation, ScreenStationaryLocation);

	int32 SizeX, SizeY;
	PC->GetViewportSize(SizeX, SizeY);
	ScreenStationaryLocation.X /= SizeX;
	ScreenStationaryLocation.Y /= SizeY;

	return ScreenStationaryLocation;
}


// Called to bind functionality to input
void AVRCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(TEXT("Teleport"), IE_Released, this, &AVRCharacter::BeginTeleport);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Pressed, this, &AVRCharacter::GripLeft);
	PlayerInputComponent->BindAction(TEXT("GripLeft"), IE_Released, this, &AVRCharacter::ReleaseRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Pressed, this, &AVRCharacter::GripRight);
	PlayerInputComponent->BindAction(TEXT("GripRight"), IE_Released, this, &AVRCharacter::ReleaseRight);

	PlayerInputComponent->BindAxis(TEXT("Forward"), this, &AVRCharacter::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("Right"), this, &AVRCharacter::MoveRight);
}

void AVRCharacter::MoveForward(float throttle)
{
	//UE_LOG(LogTemp, Warning, TEXT("MoveForward %f"), throttle);

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("MoveForward %f"), throttle));
	AddMovementInput(throttle * Camera->GetForwardVector());
}

void AVRCharacter::MoveRight(float throttle)
{
	AddMovementInput(throttle * Camera->GetRightVector());
}

void AVRCharacter::GripLeft()
{
	LeftController->Grip();
}

void AVRCharacter::ReleaseLeft()
{
	LeftController->Release();
}

void AVRCharacter::GripRight()
{
	RightController->Grip();
}

void AVRCharacter::ReleaseRight()
{
	RightController->Release();
}

void AVRCharacter::BeginTeleport()
{
	StartFade(0.0f, 1.0f);

	FTimerHandle Handle;
	GetWorld()->GetTimerManager().SetTimer(Handle, this, &AVRCharacter::FinishTeleport, TeleportFadeTime);
}

void AVRCharacter::FinishTeleport()
{
	float halfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	FVector NewPosition = DestinationMarker->GetComponentLocation() + GetActorUpVector() * halfHeight;

	SetActorLocation(NewPosition);

	StartFade(1.0f, 0.0f);
}

void AVRCharacter::StartFade(float FromAlpha, float ToAlpha)
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	if (PC != nullptr)
	{
		PC->PlayerCameraManager->StartCameraFade(FromAlpha, ToAlpha, TeleportFadeTime, FLinearColor::Black);
	}
}
