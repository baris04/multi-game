#pragma once

#include "CoreMinimal.h"
#include "Characters/MultiGameCharacter.h"
#include "PlayerCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCombatComponent;
class UAbilityComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class USkeletalMesh;
class USkeletalMeshComponent;
struct FInputActionValue;
struct FCharacterAppearance;

/**
 * Player-controlled hero: third-person camera, Enhanced Input, melee combo,
 * projectile abilities, sprint/dodge, and replicated appearance customization.
 */
UCLASS()
class MULTIGAME_API APlayerCharacter : public AMultiGameCharacter
{
	GENERATED_BODY()

public:
	APlayerCharacter();

	virtual void Tick(float DeltaSeconds) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Applies replicated appearance (mesh selection + material tints). */
	UFUNCTION(BlueprintCallable, Category = "MultiGame|Customization")
	void ApplyAppearance(const FCharacterAppearance& Appearance);

	virtual FVector GetAbilityMuzzleLocation() const override;
	virtual FVector GetAbilityAimDirection(const FVector& TargetLocation) const override;

protected:
	virtual void BeginPlay() override;
	virtual void OnDeathEffects() override;

	// --- Input handlers ---
	void Input_Move(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value);
	void Input_SprintStart(const FInputActionValue& Value);
	void Input_SprintStop(const FInputActionValue& Value);
	void Input_Dodge(const FInputActionValue& Value);
	void Input_LightAttack(const FInputActionValue& Value);
	void Input_HeavyAttack(const FInputActionValue& Value);
	void Input_Ability1(const FInputActionValue& Value);
	void Input_Ability2(const FInputActionValue& Value);

	FVector GetAbilityTargetLocation() const;
	void RefreshAppearanceFromPlayerState();
	void EnsureInputContext();

	/** Builds Input Actions + a Mapping Context in C++ so no input assets are needed. */
	void BuildDefaultInput();

	// --- Server RPCs (client -> server authoritative actions) ---
	UFUNCTION(Server, Reliable)
	void ServerLightAttack();

	UFUNCTION(Server, Reliable)
	void ServerHeavyAttack();

	UFUNCTION(Server, Reliable)
	void ServerCastAbility(int32 AbilityIndex, FVector TargetLocation);

	UFUNCTION(Server, Reliable)
	void ServerSetSprint(bool bNewSprinting);

	UFUNCTION(Server, Reliable)
	void ServerDodge(FVector DodgeDirection);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastDodge(FVector DodgeDirection);

	void EndDodgeInvulnerability();

	// --- Components ---
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MultiGame|Components")
	TObjectPtr<UCombatComponent> CombatComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MultiGame|Components")
	TObjectPtr<UAbilityComponent> AbilityComponent;

	/** Optional secondary mesh (head/hair) that follows the body pose. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MultiGame|Customization")
	TObjectPtr<USkeletalMeshComponent> HeadMesh;

	// --- Enhanced Input assets (assign in the derived Blueprint) ---
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> LightAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Ability1Action;

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> Ability2Action;

	// --- Customization data (mesh option tables) ---
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Customization")
	TArray<TObjectPtr<USkeletalMesh>> BodyMeshOptions;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Customization")
	TArray<TObjectPtr<USkeletalMesh>> HeadMeshOptions;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Customization")
	FName PrimaryColorParam = TEXT("PrimaryColor");

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Customization")
	FName SecondaryColorParam = TEXT("SecondaryColor");

	// --- Movement tuning ---
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Movement")
	float WalkSpeed = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Movement")
	float SprintSpeed = 850.f;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Movement")
	TObjectPtr<UAnimMontage> DodgeMontage;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Movement")
	float DodgeImpulse = 900.f;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Movement")
	float DodgeInvulnerabilitySeconds = 0.5f;

	bool bIsSprinting = false;
	bool bIsDodging = false;

	FTimerHandle DodgeIFrameTimer;
	bool bAppearanceApplied = false;
};
