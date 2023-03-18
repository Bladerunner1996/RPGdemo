// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Arrow.generated.h"

class UProjectileMovementComponent;
class USphereComponent;
class UBoxComponent;

UCLASS()
class RPGPRAC_API AArrow : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AArrow();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		UProjectileMovementComponent* ProjectileMovement;

	TArray<AActor*> IgnoreActors;

protected:

	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	USceneComponent* DefaultSceneRoot;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(EditAnywhere)
		USphereComponent* CollisionComp;

	void ExecuteGetHit(FHitResult& BoxHit);

	UFUNCTION(BlueprintImplementableEvent)
		void CreateFields(const FVector& FieldLocation);

	void BoxTrace(FHitResult& BoxHit);


	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
		UBoxComponent* WeaponBox;
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		bool bShowBoxDebug = false;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* BoxTraceStart;

	UPROPERTY(VisibleAnywhere)
		USceneComponent* BoxTraceEnd;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
		float Damage = 20.f;

	UFUNCTION(BlueprintCallable)
		void OnHit(UPrimitiveComponent* HitComp,
			AActor* OtherActor,
			UPrimitiveComponent* OtherComp,
			FVector NormalImpulse,
			const FHitResult& Hit);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase* HitSound;


public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
	FORCEINLINE UBoxComponent* GetWeaponBox() const { return WeaponBox; }
	UStaticMeshComponent* GetArrowMesh() { return ArrowMesh; }
	UProjectileMovementComponent* GetProjectileMovement() { return ProjectileMovement; };
	USphereComponent* GetSphere() {return  CollisionComp;}
};
