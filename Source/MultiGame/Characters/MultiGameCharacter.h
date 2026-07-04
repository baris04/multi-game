#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Core/MultiGameTypes.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "MultiGameCharacter.generated.h"

class UHealthComponent;
class UAnimMontage;
class UStaticMeshComponent;
class AMultiGameCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCharacterDied, AMultiGameCharacter*, Character);

/**
 * Shared base for players and enemies: health, team, and replicated montage playback.
 */
UCLASS(Abstract)
class MULTIGAME_API AMultiGameCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMultiGameCharacter();

	UPROPERTY(BlueprintAssignable, Category = "MultiGame|Character")
	FOnCharacterDied OnCharacterDied;

	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	UHealthComponent* GetHealthComponent() const { return HealthComponent; }

	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	ETeam GetTeam() const { return Team; }

	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	bool IsDead() const;

	/** Aim origin for abilities (camera for players, actor center for enemies). */
	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	virtual FVector GetAbilityMuzzleLocation() const;

	/** Aim direction for abilities toward a world target. */
	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	virtual FVector GetAbilityAimDirection(const FVector& TargetLocation) const;

	/** Returns true if the other character is on a hostile team. */
	UFUNCTION(BlueprintPure, Category = "MultiGame|Character")
	bool IsHostileTo(const AMultiGameCharacter* Other) const;

	UStaticMeshComponent* GetPlaceholderMesh() const { return PlaceholderMesh; }

	/** Assigns skeletal mesh + optional idle loop and hides the cylinder fallback. */
	void InitializeCharacterVisuals(USkeletalMesh* BodyMesh, class UAnimSequence* IdleAnim, class UMaterialInterface* BodyMaterial = nullptr);

	/** Register asset paths to load on BeginPlay (use after importing mannequins). */
	void SetCharacterVisualAssetPaths(
		const TArray<FString>& MeshPaths,
		const TArray<FString>& IdleAnimPaths,
		const TArray<FString>& MoveAnimPaths,
		const TArray<FString>& MaterialPaths = {});

	void SetVisualPreset(MultiGameMannequin::EVisualPreset Preset) { VisualPreset = Preset; }

	/** Loads registered paths; safe to call after Content import. */
	void EnsureCharacterVisualsLoaded();

	/** Resumes idle/walk based on current movement speed. */
	void ResumeLocomotionAnimation();

	/** Restarts idle (legacy name — forwards to locomotion resume). */
	void RestartIdleAnimation() { ResumeLocomotionAnimation(); }

	/** Optional tint applied on top of mannequin materials (mostly for enemies). */
	void ApplyCharacterTint(const FLinearColor& TintColor);

	MultiGameMannequin::EVisualPreset GetVisualPreset() const { return VisualPreset; }

	/** Server -> all: play a montage everywhere so all clients see the animation. */
	UFUNCTION(NetMulticast, Unreliable, Category = "MultiGame|Character")
	void MulticastPlayMontage(UAnimMontage* Montage, FName SectionName);

	/** Server -> all: play a raw animation sequence without requiring an AnimBlueprint. */
	UFUNCTION(NetMulticast, Unreliable, Category = "MultiGame|Character")
	void MulticastPlayVisualAnimation(class UAnimSequence* Animation, bool bLooping);

	/** Returns the character to its configured idle loop after a one-shot animation. */
	UFUNCTION(NetMulticast, Unreliable, Category = "MultiGame|Character")
	void MulticastResumeIdleAnimation();

	/** Spawns a visible projectile on clients (server already spawned the authoritative one). */
	UFUNCTION(NetMulticast, Unreliable, Category = "MultiGame|Character")
	void MulticastSpawnProjectileVisual(FVector SpawnLocation, FVector FlyDirection, float InDamage);

	/** Short melee swing flash visible on all machines. */
	UFUNCTION(NetMulticast, Unreliable, Category = "MultiGame|Character")
	void MulticastSpawnMeleeFlash(FVector Location, float Radius);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	virtual void HandleDeath(AActor* DeadActor, AActor* Killer);

	/** Overridable hook so subclasses react to death (ragdoll, AI stop, scoring). */
	virtual void OnDeathEffects();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MultiGame|Components")
	TObjectPtr<UHealthComponent> HealthComponent;

	/** Visible placeholder body used only if mannequin assets fail to load. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MultiGame|Components")
	TObjectPtr<UStaticMeshComponent> PlaceholderMesh;

	/** Tint applied to the fallback cylinder (overridden per class/team). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MultiGame|Character")
	FLinearColor PlaceholderColor = FLinearColor(0.1f, 0.4f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MultiGame|Character")
	ETeam Team = ETeam::Players;

private:
	void SetupPlaceholderMesh();
	void UpdatePlaceholderVisibility();
	void UpdateLocomotionAnimation();

	TArray<FString> PendingMeshPaths;
	TArray<FString> PendingIdleAnimPaths;
	TArray<FString> PendingMoveAnimPaths;
	TArray<FString> PendingMaterialPaths;
	TObjectPtr<class UAnimSequence> CurrentIdleAnimation;
	TObjectPtr<class UAnimSequence> CurrentMoveAnimation;
	TObjectPtr<class UAnimSequence> CurrentPlayingAnimation;
	MultiGameMannequin::EVisualPreset VisualPreset = MultiGameMannequin::EVisualPreset::Player;
	bool bCharacterVisualsApplied = false;
	bool bLocomotionPaused = false;
	float MoveAnimReferenceSpeed = 380.f;
	static constexpr float LocomotionStartSpeed = 25.f;
};
