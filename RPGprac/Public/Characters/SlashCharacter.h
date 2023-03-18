// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "InputActionValue.h"
#include "CharacterTypes.h"
#include "Interfaces/PickUpInterface.h"
#include "SlashCharacter.generated.h"

class UInputMappingContext;
class UInputAction;

class USpringArmComponent;
class UCameraComponent;
class UGroomComponent;
class AItem;
class ASoul;
class ATreasure;
class UAnimMontage;
class USlashOverlay;
class UNiagaraComponent;
class AArrow;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FEquipItemDelegate, int32, CurrentSlotIndex, int32, NewSlotIndex);
DECLARE_MULTICAST_DELEGATE(FDieDelegate);

UCLASS()
class RPGPRAC_API ASlashCharacter : public ABaseCharacter, public IPickUpInterface
{
	GENERATED_BODY()

public:

	ASlashCharacter();
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Jump() override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	virtual void SetOverlappingItem(AItem* Item) override;
	virtual void AddSouls(ASoul* Soul) override;
	virtual void AddGold(ATreasure* Treasure) override;
	virtual void AddBlood(ABlood* Blood) override;

	float CameraCurrentFOV=90.f;
	float CameraZoomedFOV = 60.f;
	float CameraDefaultFOV = 90.f;
	void CameraInterpZoom(float DeltaTime);
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return ViewCamera; }

	//UPROPERTY()
	FDieDelegate DieDelegate;

protected:

	virtual void BeginPlay() override;

	UFUNCTION()
	void EnablePPV(float val);

	UFUNCTION()
	void TurnGray();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputMappingContext* SlashContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	TSubclassOf<UUserWidget> CrossHairAsset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HUD")
	UUserWidget* CrossHair;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* MovementAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) //改变镜头
	UInputAction* CameraAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input) 
	UInputAction* EquipAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* AttackAction2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* DodgeAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UNiagaraComponent* DashEffect;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Weapon)
	TSubclassOf<AArrow> ArrowClass;

	virtual int32 PlayAttackMontage() override;
	virtual void AttackEnd() override;
	virtual void DodgeEnd() override;
	virtual bool CanAttack() override;
	bool CanDisarm();
	bool CanArm();
	void PlayEquipMontage(const FName& SectionName);
	void PlayBowDodgeMontage(const FName& SectionName);
	virtual void Die_Implementation() override;   //虚函数重载时也要重载Implementation的版本。

	bool HasEnoughStamina();
	bool IsOccupied();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToBack();

	UFUNCTION(BlueprintCallable)
	void AttachWeaponToHand();

	UFUNCTION(BlueprintCallable)
	void WhenDrawBow();

	UFUNCTION(BlueprintCallable)
	void WhenLooseArrow();

	void ArrowLineTrace(FHitResult& BoxHit);

	UFUNCTION(BlueprintCallable)
	void FinishEquiping();
	
	UFUNCTION(BlueprintCallable)
	void HitReactEnd();

	UFUNCTION()
		void ChangeControllerMode();

	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void CameraChange(const FInputActionValue& Value);//改变镜头
	virtual void Attack() override;
	void Attack2();
	virtual void Dodge();

	UFUNCTION()
	void TimeStop();


	UFUNCTION()
		void HeartBoom();
	FTimerHandle HeartBoomTimer;

	void CameraInterpHeart(float DeltaTime);

	bool bHeart=false;

	UFUNCTION()
	void TimeRecover();

	FTimerHandle DilationTimer;

	bool bTimeStop=false;

	void EKeyPressed();

	void EquipWeapon(AWeapon* Weapon);

	void SwapWeapon(AWeapon* WeaponToSwap);

	/** Detach weapon and let it fall to the ground */
	void DropItemAtOne();

	//UPROPERTY(EditAnywhere, Category = Combat)     //可以设置default weapon
	//TSubclassOf<class AWeapon> WeaponClass;

	void FKeyPressed();
	void OneKeyPressed();
	void TwoKeyPressed();
	void ThreeKeyPressed();
	void FourKeyPressed();
	void FiveKeyPressed();

	void ExchangeInventoryItems(int32 CurrentItemIndex, int32 NewItemIndex);
	
	//后处理
	UPROPERTY(EditDefaultsOnly, Category = "Post Process")
		UMaterialInstance* TestMatIns = nullptr;

	UPROPERTY()
		UMaterialInstanceDynamic* TestMatInsDyna = nullptr;

	void AddDynaMatForPostProcessVolume();

private:

	void InitializeSlashOverlay();
	void SetHUDHealth();

	ECharacterState CharacterState = ECharacterState::ECS_Unequipped;

	EActionState ActionState = EActionState::EAS_Unoccupied;

	UPROPERTY(VisibleAnywhere)
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* ViewCamera;

	UPROPERTY(VisibleAnywhere,Category = Hair)
	UGroomComponent* Hair;

	UPROPERTY(VisibleAnywhere, Category = Hair)
	UGroomComponent* Eyebrows;

	UPROPERTY(VisibleInstanceOnly)
	AItem* OverlappingItem;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* EquipMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
		UAnimMontage* BowAttackMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
		UAnimMontage* AttackMontage2;

	UPROPERTY()
	USlashOverlay* SlashOverlay;

	//An array of AItems for our Inventory
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Inventory, meta = (AllowPrivateAccess = "true"))
		TArray<AItem*> Inventory;

	const int32 INVENTORY_CAPACITY{ 6 };

	/** Delegate for sending slot information to InventoryBar when equipping */
	UPROPERTY(BlueprintAssignable, Category = Delegates, meta = (AllowPrivateAccess = "true"))
		FEquipItemDelegate EquipItemDelegate;

	bool bBowStringAttachToHand = false;

	AArrow* Arrow;

	UPROPERTY(EditAnywhere, Category = "Sound")
	USoundBase* StringSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundBase* HeartSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
		USoundBase* ZoomSound;
public:

	FORCEINLINE ECharacterState GetCharacterState() const { return CharacterState; }
	FORCEINLINE EActionState GetActionState() const { return ActionState; }

	UFUNCTION(BlueprintCallable)
	bool GetBowStringIsAttachToHand() { return bBowStringAttachToHand; }
};
