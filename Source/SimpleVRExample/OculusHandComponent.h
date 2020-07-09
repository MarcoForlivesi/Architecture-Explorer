// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MotionControllerComponent.h"
#include "OculusHandComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class SIMPLEVREXAMPLE_API UOculusHandComponent : public UMotionControllerComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UOculusHandComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
protected:

	virtual void OnRegister() override;

	// Called when the game starts
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* MeshController;
};
