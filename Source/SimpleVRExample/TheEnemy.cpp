// Fill out your copyright notice in the Description page of Project Settings.


#include "TheEnemy.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

// Sets default values
ATheEnemy::ATheEnemy()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComponent"));
	RootComponent = BoxComponent;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(BoxComponent);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetEnableGravity(false);
}

// Called when the game starts or when spawned
void ATheEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	auto PlayerPawn = UGameplayStatics::GetPlayerPawn(this, 0);
	FVector Direction = PlayerPawn->GetActorLocation() - GetActorLocation();
	Direction = UKismetMathLibrary::Normal(Direction);

	Mesh->AddImpulse(AttackStrength * Direction);
}

// Called every frame
void ATheEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

