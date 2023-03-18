// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Item.h"
#include "Components/SphereComponent.h"
#include "Interfaces/PickUpInterface.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AItem::AItem()
{
	PrimaryActorTick.bCanEverTick = true;
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMeshComponent"));
	ItemMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	ItemMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = ItemMesh;

	Sphere = CreateDefaultSubobject<USphereComponent>(TEXT("Sphere"));
	Sphere->SetupAttachment(GetRootComponent()); 
	ItemEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("Embers"));
	ItemEffect->SetupAttachment(GetRootComponent());
	SlotIndex=0;
}


void AItem::BeginPlay()
{
	Super::BeginPlay();

	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnSphereOverlap);
	Sphere->OnComponentEndOverlap.AddDynamic(this, &AItem::OnSphereEndOverlap);
	//set customDepth to disabled
	InitializeCustomDepth();
}

float AItem::TransformedSin()
{
	return Amplitude * FMath::Sin(RunningTime * TimeConstant);
}

float AItem::TransformedCos()
{
	return Amplitude * FMath::Cos(RunningTime * TimeConstant);
}

void AItem::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	EnableCustomDepth();
	IPickUpInterface* PickUpInterface = Cast<IPickUpInterface>(OtherActor);
	if (PickUpInterface)
	{
		PickUpInterface->SetOverlappingItem(this);
	}
}

void AItem::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	DisableCustomDepth();
	IPickUpInterface* PickUpInterface = Cast<IPickUpInterface>(OtherActor);
	if (PickUpInterface)
	{
		PickUpInterface->SetOverlappingItem(nullptr);
	}
}

void AItem::SpawnPickUpSystem()
{
	if (PickUpEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickUpEffect,
			GetActorLocation()
		);
	}
}

void AItem::SpawnPickUpSound()
{
	if (PickUpSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			PickUpSound,
			GetActorLocation()
		);
	}
}

void AItem::EnableCustomDepth()
{
	ItemMesh->SetRenderCustomDepth(true);
}

void AItem::DisableCustomDepth()
{
	ItemMesh->SetRenderCustomDepth(false);
}
void AItem::SetItemState(EItemState State)
{
	ItemState = State;
	if (State == EItemState::EIS_PickedUp) {
		ItemMesh->SetVisibility(false);
	}
	if (State == EItemState::EIS_Equipped) {
		ItemMesh->SetVisibility(true);
	}
}

void AItem::InitializeCustomDepth()
{
	DisableCustomDepth();
}


void AItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RunningTime += DeltaTime;

	if (ItemState == EItemState::EIS_Hovering)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, TransformedSin()));
		AddActorWorldRotation(FRotator(0.f, 3 * FMath::Sin(RunningTime * 0.5), 0.f));
	}
	

}

