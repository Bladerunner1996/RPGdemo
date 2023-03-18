// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/SlashCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GroomComponent.h"
#include "Components/AttributeComponent.h"
#include "Components/SphereComponent.h"
#include "NiagaraComponent.h"
#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Kismet/GameplayStatics.h"
//#include "Items/Weapons/Bow.h"
#include "Items/Weapons/Arrow.h"
#include "Animation/AnimMontage.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "Items/Blood.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"
//EnhancedInput
#include "Components/InputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"

#include "GameFramework/ProjectileMovementComponent.h"
#include "Engine/PostProcessVolume.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "TimerManager.h"
//#include "GameplayCueNotifyTypes.h"

ASlashCharacter::ASlashCharacter()
{

	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	DashEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DashEffect"));
	DashEffect->SetupAttachment(GetMesh());
	DashEffect->bAutoActivate = false;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(GetRootComponent());
	SpringArm->TargetArmLength = 450.f;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(SpringArm);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");

}



void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	//EnhancedInputLocalPlayerSubsystem
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashContext, 0);
		}
	}
	if (CrossHairAsset) {
		CrossHair = CreateWidget<UUserWidget>(GetWorld(), CrossHairAsset);
		CrossHair->AddToViewport();
		CrossHair->SetVisibility(ESlateVisibility::Hidden);
	}
	Tags.Add(FName("EngageableTarget"));
	//指定context和优先级
	InitializeSlashOverlay();
	//绑定die委托
	DieDelegate.AddLambda([]{UE_LOG(LogTemp, Warning, TEXT("Died"))});

	//PostProcessVolume
	EnablePPV(0.f);
}

void ASlashCharacter::EnablePPV(float val)
{
	UWorld* World = GetWorld();
	if (World && !World->PostProcessVolumes.IsEmpty()) {
		auto Iterface_PostVolume = GetWorld()->PostProcessVolumes.Last(0);
		APostProcessVolume* PostVolume = Cast<APostProcessVolume>(Iterface_PostVolume);
		FPostProcessSettings& PostProcessSettings = PostVolume->Settings;
		auto& K = PostProcessSettings.WeightedBlendables.Array.Last(0);
		K.Weight = val;
	}
}

void ASlashCharacter::AddDynaMatForPostProcessVolume()
{
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, APostProcessVolume::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		APostProcessVolume* PPV = Cast<APostProcessVolume>(Actors[0]);
		if (PPV)
		{
			FPostProcessSettings& PostProcessSettings = PPV->Settings;
			if (TestMatIns)
			{
				TestMatInsDyna = UKismetMaterialLibrary::CreateDynamicMaterialInstance(this, TestMatIns);
				FWeightedBlendable WeightedBlendable;
				WeightedBlendable.Object = TestMatInsDyna;
				WeightedBlendable.Weight = 1;

				PostProcessSettings.WeightedBlendables.Array.Add(WeightedBlendable);
			}
		}
	}
}

void ASlashCharacter::TurnGray()
{
	if (SlashOverlay) {
		SlashOverlay->SetVisibility(ESlateVisibility::Hidden);
	}
	//屏幕变灰
}

void ASlashCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CameraInterpZoom(DeltaTime);
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

}

void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}

void ASlashCharacter::AddSouls(ASoul* Soul)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}

void ASlashCharacter::AddGold(ATreasure* Treasure)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}

void ASlashCharacter::AddBlood(ABlood* Blood)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->AddBlood(Blood->GetBlood());
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());;
	}
}

void ASlashCharacter::CameraInterpZoom(float DeltaTime)
{
	//弓的视角
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("HandBowSocket"))
	{
		// Interpolate to zoomed FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraZoomedFOV,
			DeltaTime,
			5);
	}
	else
	{
		// Interpolate to default FOV
		CameraCurrentFOV = FMath::FInterpTo(
			CameraCurrentFOV,
			CameraDefaultFOV,
			DeltaTime,
			10);
	}
	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}




void ASlashCharacter::Jump()
{
	if (ActionState == EActionState::EAS_Unoccupied)
	{
		Super::Jump();
	}
	bBowStringAttachToHand = false;

}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	
	//if (ActionState != EActionState::EAS_Unoccupied) return; //如果正在攻击动画，不能移动
	if (ActionState != EActionState::EAS_Unoccupied 
		&& ActionState != EActionState::EAS_EquippingWeapon
		&& ActionState != EActionState::EAS_CrazyArrow) return;
	
	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller)
	{
		const FRotator ControlRotation = GetControlRotation();
		const FRotator YawRotation(0.f, ControlRotation.Yaw, 0.f);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(ForwardDirection, MovementVector.Y);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisValue = Value.Get<FVector2D>();
	if (GetController())
	{
		AddControllerYawInput(LookAxisValue.X);
		AddControllerPitchInput(LookAxisValue.Y);
	}
}

void ASlashCharacter::CameraChange(const FInputActionValue& Value)
{
	const float Flag = Value.Get<float>();
	//UE_LOG(LogTemp, Warning, TEXT("shubiao:%f"), Flag);
	float length = SpringArm->TargetArmLength;
	length -= Flag * 7.f;
	if (length > 1000.f || length < 100.f) return;
	else SpringArm->TargetArmLength = length;
	
}

void ASlashCharacter::Attack()
{
	Super::Attack();
	if (CanAttack())
	{
		PlayAttackMontage();
		//ActionState = EActionState::EAS_Attacking;
		if (Attributes && SlashOverlay)
		{
			Attributes->GainStamina(20.f);
			SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

void ASlashCharacter::Attack2()
{
	Super::Attack();
	if (CanAttack() && Attributes->GetStamina() > 30.f)
	{
		if (EquippedWeapon->GetSKMesh()) {
			ActionState = EActionState::EAS_CrazyArrow;
			PlayMontageSection(BowAttackMontage, FName("Default"));  //弓的移动连击
		}
		else {
			ActionState = EActionState::EAS_Attacking;
			PlayMontageSection(AttackMontage2, FName("Default")); //剑的连击动画
		}
		if (Attributes && SlashOverlay)
		{
			Attributes->UseStamina(30.f);
			SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		}
	}
}

void ASlashCharacter::Dodge()
{
	if (IsOccupied() || !HasEnoughStamina()) return;
	if (ActionState != EActionState::EAS_Unoccupied) return;
	if (GetCharacterMovement()->IsFalling()) return;
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("HandBowSocket")) 
	{
		PlayBowDodgeMontage(FName("Bow"));
	}
	else
		PlayDodgeMontage();
	ActionState = EActionState::EAS_Dodge;
	if (Attributes && SlashOverlay)
	{
		Attributes->UseStamina(Attributes->GetDodgeCost());
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}
}

void ASlashCharacter::TimeStop()
{
	if (!bTimeStop && Attributes->GetStamina()>40) {
		Attributes->UseStamina(40.f);
		//开启径向模糊
		EnablePPV(1.f);
		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.25f);
		bTimeStop = true;
		CustomTimeDilation = 3.f;
		//屏幕跳动
		GetWorldTimerManager().SetTimer(HeartBoomTimer, this, &ASlashCharacter::HeartBoom, 0.26f, true);
		//时停时长，2/0.25=8
		GetWorldTimerManager().SetTimer(DilationTimer, this, &ASlashCharacter::TimeRecover, 2.f);
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ZoomSound, this->GetActorLocation());
		UGameplayStatics::PlaySound2D(GetWorld(), HeartSound, 1.f, 1.f, 8.1f);
	}
	
}

void ASlashCharacter::HeartBoom()
{
	auto Now = FDateTime::Now();
	UE_LOG(LogTemp, Warning, TEXT("MonitorPlayer Time:%d %d %d"), Now.GetMinute(), Now.GetSecond(), Now.GetMillisecond());
	float CurFOV = GetFollowCamera()->FieldOfView;
	if (bHeart == false) {
		CurFOV = FMath::FInterpTo(
			CurFOV,
			CurFOV - 10.f,
			0.2,
			10);
		bHeart = true;
		UE_LOG(LogTemp, Warning, TEXT("p"));
	}
	else {
		CurFOV = FMath::FInterpTo(
			CurFOV,
			CurFOV + 10.f,
			0.2,
			10);
		bHeart = false;
		UE_LOG(LogTemp, Warning, TEXT("q"));
	}
	UE_LOG(LogTemp, Warning, TEXT("FOV: %f"), CurFOV);
	GetFollowCamera()->SetFieldOfView(80);
}


void ASlashCharacter::TimeRecover()
{
	GetWorldTimerManager().ClearTimer(HeartBoomTimer);
	EnablePPV(0.f);
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.f);
	bTimeStop = false;
	CustomTimeDilation = 1.f;
}


int32 ASlashCharacter::PlayAttackMontage()
{
	ActionState = EActionState::EAS_Attacking;
	if (EquippedWeapon->GetSKMesh()) {
		PlayMontageSection(AttackMontage, FName("Fire1"));
		return 0;
	}
	else {
		return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
	}
}

void ASlashCharacter::AttackEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::DodgeEnd()
{
	Super::DodgeEnd();

	ActionState = EActionState::EAS_Unoccupied;
}

bool ASlashCharacter::CanAttack()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped;
}



bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && CharacterState != ECharacterState::ECS_Unequipped ;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied 
		&& CharacterState == ECharacterState::ECS_Unequipped
		&& EquippedWeapon;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("RightHandSocket"))
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
	//如果是弓
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("HandBowSocket"))
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineBowSocket"));
		
		if (Arrow && Arrow->IsAttachedTo(EquippedWeapon)) {
			Arrow->Destroy();
		}
	}
	ChangeControllMode();
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("SpineSocket"))
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("SpineBowSocket"))
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("HandBowSocket"));
	}
	ChangeControllMode();
}

void ASlashCharacter::WhenDrawBow()
{
	bBowStringAttachToHand = true;
}

void ASlashCharacter::WhenLooseArrow()
{
	if (Arrow) {
		Arrow->Destroy();
	}
	bBowStringAttachToHand = false;
	//Line Trace part
	//就用camera location代替crosshair了
	const FVector CameraLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	FVector CameraForwardVector = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector();
	FVector TraceEndLocation = CameraLocation + 20000 * CameraForwardVector;

	FHitResult BoxHit;
	ArrowLineTrace(BoxHit);
	if (BoxHit.bBlockingHit) {
		TraceEndLocation = BoxHit.Location;
	}

	//生成一只从string socket到Trace End的弓
	auto BowMesh = EquippedWeapon->GetSKMeshComponent();
	FTransform Transform = BowMesh->GetSocketTransform(FName("BowStringSocket"), ERelativeTransformSpace::RTS_World);
	FVector ArrowSpawnLocation = Transform.GetLocation();
	FRotator ArrowSpawnRotation = UKismetMathLibrary::MakeRotFromX(TraceEndLocation - ArrowSpawnLocation);

	FActorSpawnParameters Parameters = FActorSpawnParameters();
	Parameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AArrow* SpawnedArrow = GetWorld()->SpawnActor<AArrow>(ArrowClass, ArrowSpawnLocation,ArrowSpawnRotation, Parameters);


	SpawnedArrow->SetOwner(this);
	SpawnedArrow->SetInstigator(this);


	auto Projectile = SpawnedArrow->GetProjectileMovement();
	Projectile->ProjectileGravityScale = 1;
	//Projectile->Velocity = 3000 * Direction;

	if (StringSound) {
		UGameplayStatics::PlaySoundAtLocation(
			this,
			StringSound,
			EquippedWeapon->GetActorLocation()
		);
	}
}

void ASlashCharacter::ArrowLineTrace(FHitResult& BoxHit)
{
	UE_LOG(LogTemp, Warning, TEXT("Trace"));
	//LineTrace Start location
	const FVector CameraLocation = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetCameraLocation();
	//End location
	FVector CameraForwardVector = GetWorld()->GetFirstPlayerController()->PlayerCameraManager->GetActorForwardVector();
	const FVector TraceEndLocation = CameraLocation + 20000 * CameraForwardVector;

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	GetWorld()->LineTraceSingleByChannel(
		BoxHit,
		CameraLocation,
		TraceEndLocation,
		ECollisionChannel::ECC_Visibility
	);
}


void ASlashCharacter::Die_Implementation()
{
	Super::Die_Implementation();

	ActionState = EActionState::EAS_Dead;
	DisableMeshCollision();
	//镜头拉远，画面变灰。
	DieDelegate.Broadcast();

}

bool ASlashCharacter::HasEnoughStamina()
{
	return Attributes && Attributes->GetStamina() > Attributes->GetDodgeCost();
}

bool ASlashCharacter::IsOccupied()
{
	return ActionState != EActionState::EAS_Unoccupied;
}


void ASlashCharacter::FinishEquiping()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
	UE_LOG(LogTemp,Warning, TEXT("Call Hit React End"));
}



void ASlashCharacter::PlayEquipMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && EquipMontage)
	{
		AnimInstance->Montage_Play(EquipMontage);
		AnimInstance->Montage_JumpToSection(SectionName, EquipMontage);
	}
}

void ASlashCharacter::PlayBowDodgeMontage(const FName& SectionName)
{
	bBowStringAttachToHand = false;
	PlayMontageSection(DodgeMontage, FName("Bow"));
}


void ASlashCharacter::ChangeControllMode()
{
	if (EquippedWeapon && EquippedWeapon->GetAttachParentSocketName() == FName("HandBowSocket"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Bow111"));
		bUseControllerRotationYaw = true;
		GetCharacterMovement()->bOrientRotationToMovement = false;
		CrossHair->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ori222"));
		bUseControllerRotationYaw = false;
		GetCharacterMovement()->bOrientRotationToMovement = true;
		CrossHair->SetVisibility(ESlateVisibility::Hidden);
	}
	
}

void ASlashCharacter::EKeyPressed()  //捡起武器
{
	
	AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem);
	//如果重合的是武器AWeapon
	if (OverlappingWeapon)  
	{
		//如果背包没满
		if (Inventory.Num() < INVENTORY_CAPACITY) {
			//如果身上有武器
			if (EquippedWeapon) {
				OverlappingWeapon->SetSlotIndex(Inventory.Num());
				Inventory.Add(OverlappingWeapon);
				OverlappingWeapon->PlayEquipSound();  //
				OverlappingWeapon->SetItemState(EItemState::EIS_PickedUp);
				OverlappingWeapon->DeactivateEmbers();
				OverlappingItem = nullptr;
			}
			//如果身上没有武器
			else {
				EquipWeapon(OverlappingWeapon);
				OverlappingWeapon->SetSlotIndex(Inventory.Num());
				Inventory.Add(OverlappingWeapon);
				OverlappingWeapon->PlayEquipSound();  //
				OverlappingWeapon->SetItemState(EItemState::EIS_Equipped);
			}
		}
		//如果背包满了
		else {
			//swap weapon
		}
	}
	else
	{
		if (CanDisarm())
		{
			if (!EquippedWeapon->GetSKMesh()) {
				PlayEquipMontage(FName("Unequip"));
			}
			else {
				PlayEquipMontage(FName("UnequipBow"));
				bBowStringAttachToHand = false;
				EquippedWeapon->GetItemMesh()->SetVisibility(false, false);
			}
			CharacterState = ECharacterState::ECS_Unequipped;
			ActionState = EActionState::EAS_EquippingWeapon;
			//DropWeapon();
		}
		else if (CanArm())
		{
			if (!EquippedWeapon->GetSKMesh()) {
				PlayEquipMontage(FName("Equip"));
				CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
			}
			else {
				PlayEquipMontage(FName("EquipBow"));
				CharacterState = ECharacterState::ECS_EquippedBow;
			}
			ActionState = EActionState::EAS_EquippingWeapon;
		}
	}
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	//如果有SKMesh，说明是弓
	if (Weapon->GetSKMesh()) {
		Weapon->Equip(GetMesh(), FName("HandBowSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedBow;
	}
	else {
		Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	}
	if (EquippedWeapon == nullptr)
	{
		// -1 == no EquippedWeapon yet. No need to reverse the icon animation
		EquipItemDelegate.Broadcast(-1, Weapon->GetSlotIndex());
	}
	else
	{
		EquipItemDelegate.Broadcast(EquippedWeapon->GetSlotIndex(), Weapon->GetSlotIndex());
	}
	
	OverlappingItem = nullptr;
	EquippedWeapon = Weapon;
	EquippedWeapon->SetItemState(EItemState::EIS_Equipped);
	ChangeControllMode();

}

//void ASlashCharacter::SwapWeapon(AWeapon* WeaponToSwap)  //Weapon to swap
//{
//	if (Inventory.Num() - 1 >= EquippedWeapon->GetSlotIndex())
//	{
//		Inventory[EquippedWeapon->GetSlotIndex()] = WeaponToSwap;
//		WeaponToSwap->SetSlotIndex(EquippedWeapon->GetSlotIndex());
//	}
//
//	DropItemAtOne();
//	EquipWeapon(WeaponToSwap);
//	OverlappingItem = nullptr;
//}

void ASlashCharacter::DropItemAtOne()
{
	if (EquippedWeapon)
	{
		Inventory.RemoveAt(EquippedWeapon->GetSlotIndex());
		
		GetWorld()->SpawnActor<AWeapon>(EquippedWeapon->GetClass(), GetActorLocation(),GetActorRotation());
		
		EquippedWeapon->Destroy();
		EquippedWeapon = nullptr;
		CharacterState = ECharacterState::ECS_Unequipped;
	}
}

void ASlashCharacter::FKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 0) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 0);
	}

}

void ASlashCharacter::OneKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 1) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 1);
	}

}

void ASlashCharacter::TwoKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 2) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 2);
	}

}

void ASlashCharacter::ThreeKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 3) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 3);
	}

}

void ASlashCharacter::FourKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 4) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 4);
	}

}

void ASlashCharacter::FiveKeyPressed()
{
	if (EquippedWeapon) {
		if (EquippedWeapon->GetSlotIndex() == 5) return;
		ExchangeInventoryItems(EquippedWeapon->GetSlotIndex(), 5);
	}

}

void ASlashCharacter::ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex)
{
	if ((CurrentItemIndex == NewItemIndex) || (NewItemIndex >= Inventory.Num())) return;
	auto OldEquippedWeapon = EquippedWeapon;
	auto NewWeapon = Cast<AWeapon>(Inventory[NewItemIndex]);
	EquipWeapon(NewWeapon);

	OldEquippedWeapon->SetItemState(EItemState::EIS_PickedUp);
	NewWeapon->SetItemState(EItemState::EIS_Equipped);
}


void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction("FKey", IE_Pressed, this,&ASlashCharacter::FKeyPressed);
	PlayerInputComponent->BindAction("1Key", IE_Pressed, this,&ASlashCharacter::OneKeyPressed);
	PlayerInputComponent->BindAction("2Key", IE_Pressed, this,&ASlashCharacter::TwoKeyPressed);
	PlayerInputComponent->BindAction("3Key", IE_Pressed, this,&ASlashCharacter::ThreeKeyPressed);
	PlayerInputComponent->BindAction("4Key", IE_Pressed, this,&ASlashCharacter::FourKeyPressed);
	PlayerInputComponent->BindAction("5Key", IE_Pressed, this,&ASlashCharacter::FiveKeyPressed);
	PlayerInputComponent->BindAction("CtrlE", IE_Pressed, this, &ASlashCharacter::DropItemAtOne);
	//时停特效
	PlayerInputComponent->BindAction("RKey", IE_Pressed, this, &ASlashCharacter::TimeStop);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MovementAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(CameraAction, ETriggerEvent::Triggered, this, &ASlashCharacter::CameraChange);
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EKeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack);
		EnhancedInputComponent->BindAction(AttackAction2, ETriggerEvent::Triggered, this, &ASlashCharacter::Attack2);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Dodge);
	}
}

float ASlashCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	SetHUDHealth();
	return DamageAmount;
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter);
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	if (Attributes && Attributes->GetHealthPercent() > 0.f)
	{
		ActionState = EActionState::EAS_HitReaction;
	}
}


void ASlashCharacter::InitializeSlashOverlay()
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController)
	{
		ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD());
		if (SlashHUD)
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}
}

void ASlashCharacter::SetHUDHealth()
{
	if (SlashOverlay && Attributes)
	{
		SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
	}
}