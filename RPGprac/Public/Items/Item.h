// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

class USphereComponent;
class UNiagaraComponent;

enum class EItemState : uint8
{
	EIS_Hovering,
	EIS_Falling,
	EIS_PickedUp,
	EIS_Equipped

};


UCLASS()
class RPGPRAC_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	AItem();
    virtual void Tick(float DeltaTime) override;

	virtual void EnableCustomDepth();
	virtual void DisableCustomDepth();
	FORCEINLINE UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }
	FORCEINLINE EItemState GetItemState() const { return ItemState; }
	virtual void SetItemState(EItemState State);

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float Amplitude = 0.25f;       //Item上下浮动的幅度

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sine Parameters")
	float TimeConstant = 5.f;

	UFUNCTION(BlueprintPure)
	float TransformedSin();

	UFUNCTION(BlueprintPure)
	float TransformedCos();

	template<typename T>
	T Avg(T First, T Second);

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
    UStaticMeshComponent* ItemMesh;

	EItemState ItemState = EItemState::EIS_Hovering;


	UFUNCTION()
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
    
	virtual void SpawnPickUpSystem();
	virtual void SpawnPickUpSound();

	UPROPERTY(VisibleAnywhere)
    USphereComponent* Sphere;

	UPROPERTY(EditAnywhere)
	UNiagaraComponent* ItemEffect;

	UPROPERTY(EditAnywhere)
	USoundBase* PickUpSound;

	
	virtual void InitializeCustomDepth();

	/** Backgroud icon for the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
		UTexture2D* IconBackground;

	/** Item icon for the inventory */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
		UTexture2D* IconItem;

	//Slot in the Inventory array
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, Category = Inventory, meta = (AllowPrivateAccess = "true"))
	int32 SlotIndex;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	float RunningTime;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickUpEffect;

public:
	FORCEINLINE int32 GetSlotIndex() const { return SlotIndex; }
	FORCEINLINE void SetSlotIndex(int32 Index) { SlotIndex = Index; }
	
};

template<typename T>
inline T AItem::Avg(T First, T Second)
{
	return (First + Second) / 2;
}
