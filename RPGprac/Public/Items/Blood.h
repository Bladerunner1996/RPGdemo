// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Blood.generated.h"

/**
 * 
 */
UCLASS()
class RPGPRAC_API ABlood : public AItem
{
	GENERATED_BODY()
protected:
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
private:
	UPROPERTY(EditAnywhere, Category = "Blood Properties")
	int32 BloodAmount;
public:
	FORCEINLINE int32 GetBlood() const { return BloodAmount; }
	FORCEINLINE void SetBlood(int32 Bloods) { BloodAmount = Bloods; }
};
