// Fill out your copyright notice in the Description page of Project Settings.


#include "OculusHandComponent.h"
#include "Components\SkeletalMeshComponent.h"

// Sets default values for this component's properties
UOculusHandComponent::UOculusHandComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void UOculusHandComponent::OnRegister()
{
#if WITH_EDITORONLY_DATA
	MeshController = NewObject<USkeletalMeshComponent>(this, TEXT("Mesh"), RF_Transactional | RF_TextExportTransient);
	MeshController->SetupAttachment(this);
#endif
	Super::OnRegister();
}

// Called when the game starts
void UOculusHandComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UOculusHandComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

