// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 *
 */
UCLASS()
class RPGPRAC_API AWeapon : public AItem
{
	GENERATED_BODY()
public:
	AWeapon();
	virtual void Tick(float DeltaTime) override;
	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator);
	void DeactivateEmbers();
	void DisableSphereCollision();
	void PlayEquipSound();
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);

	TArray<AActor*> IgnoreActors;

	void ThrowWeapon();
	void StopFalling();

	virtual void SetItemState(EItemState State) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
		void OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	bool ActorIsSameType(AActor* OtherActor);

	void ExecuteGetHit(FHitResult& BoxHit);

	UFUNCTION(BlueprintImplementableEvent)
		void CreateFields(const FVector& FieldLocation);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		USkeletalMeshComponent* SkeletalMeshCPN;

private:

	void BoxTrace(FHitResult& BoxHit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		bool bShowBoxDebug = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		USoundBase* EquipSound;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		UBoxComponent* WeaponBox;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		float Damage = 20.f;

	//Throw Weapon
	FTimerHandle ThrowWeaponTimer;
	float ThrowWeaponTime;
	bool bFalling;

public:
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	FORCEINLINE USkeletalMesh* GetSKMesh() const{ return SkeletalMeshCPN->SkeletalMesh; }
	FORCEINLINE USkeletalMeshComponent* GetSKMeshComponent() const { return SkeletalMeshCPN; }
};