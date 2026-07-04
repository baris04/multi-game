#include "Characters/MultiGameCharacter.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "Components/HealthComponent.h"
#include "Combat/ProjectileBase.h"
#include "Combat/CombatVisuals.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimSequence.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

AMultiGameCharacter::AMultiGameCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	HealthComponent = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
		MeshComp->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));
	}

	SetupPlaceholderMesh();
}

void AMultiGameCharacter::SetupPlaceholderMesh()
{
	// Engine cylinder scaled to roughly fill the capsule; purely a visual stand-in.
	PlaceholderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaceholderMesh"));
	PlaceholderMesh->SetupAttachment(GetCapsuleComponent());
	PlaceholderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PlaceholderMesh->SetRelativeLocation(FVector(0.f, 0.f, -90.f));
	PlaceholderMesh->SetRelativeScale3D(FVector(0.8f, 0.8f, 1.8f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CylinderMesh(
		TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (CylinderMesh.Succeeded())
	{
		PlaceholderMesh->SetStaticMesh(CylinderMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> BasicMat(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMat.Succeeded())
	{
		PlaceholderMesh->SetMaterial(0, BasicMat.Object);
	}
	PlaceholderMesh->SetVisibility(true, true);
}

void AMultiGameCharacter::InitializeCharacterVisuals(USkeletalMesh* BodyMesh, UAnimSequence* IdleAnim, UMaterialInterface* BodyMaterial)
{
	CurrentIdleAnimation = IdleAnim;
	MultiGameMannequin::ConfigureCharacterMesh(this, BodyMesh, IdleAnim, BodyMaterial);
	if (BodyMesh)
	{
		bCharacterVisualsApplied = true;
		CurrentPlayingAnimation = IdleAnim;
		UpdatePlaceholderVisibility();
	}
}

void AMultiGameCharacter::SetCharacterVisualAssetPaths(
	const TArray<FString>& MeshPaths,
	const TArray<FString>& IdleAnimPaths,
	const TArray<FString>& MoveAnimPaths,
	const TArray<FString>& MaterialPaths)
{
	PendingMeshPaths = MeshPaths;
	PendingIdleAnimPaths = IdleAnimPaths;
	PendingMoveAnimPaths = MoveAnimPaths;
	PendingMaterialPaths = MaterialPaths;
}

void AMultiGameCharacter::EnsureCharacterVisualsLoaded()
{
	if (bCharacterVisualsApplied)
	{
		UpdatePlaceholderVisibility();
		return;
	}

	USkeletalMesh* BodyMesh = MultiGameMannequin::LoadFirstMesh(PendingMeshPaths);
	UAnimSequence* IdleAnim = MultiGameMannequin::LoadFirstAnimSequence(PendingIdleAnimPaths);
	UAnimSequence* MoveAnim = MultiGameMannequin::LoadFirstAnimSequence(PendingMoveAnimPaths);
	UMaterialInterface* BodyMaterial = MultiGameMannequin::LoadFirstMaterial(PendingMaterialPaths);

	if (BodyMesh)
	{
		CurrentMoveAnimation = MoveAnim;
		InitializeCharacterVisuals(BodyMesh, IdleAnim, BodyMaterial);
		MultiGameMannequin::ApplyPresetMeshAdjustments(this, VisualPreset);
	}
	else
	{
		UpdatePlaceholderVisibility();
	}
}

void AMultiGameCharacter::ApplyCharacterTint(const FLinearColor& TintColor)
{
	MultiGameMannequin::ApplyMeshTint(GetMesh(), TintColor);
}

void AMultiGameCharacter::ResumeLocomotionAnimation()
{
	bLocomotionPaused = false;
	CurrentPlayingAnimation = nullptr;
	UpdateLocomotionAnimation();
}

void AMultiGameCharacter::UpdateLocomotionAnimation()
{
	if (bLocomotionPaused || !CurrentIdleAnimation || IsDead())
	{
		return;
	}

	USkeletalMeshComponent* MeshComp = GetMesh();
	if (!MeshComp || !MeshComp->GetSkeletalMeshAsset())
	{
		return;
	}

	const float Speed = GetVelocity().Size2D();
	UAnimSequence* DesiredAnim = (Speed >= LocomotionStartSpeed && CurrentMoveAnimation)
		? CurrentMoveAnimation
		: CurrentIdleAnimation;

	if (DesiredAnim != CurrentPlayingAnimation)
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComp->PlayAnimation(DesiredAnim, true);
		CurrentPlayingAnimation = DesiredAnim;
	}

	if (DesiredAnim == CurrentMoveAnimation && MoveAnimReferenceSpeed > 1.f)
	{
		MeshComp->SetPlayRate(FMath::Clamp(Speed / MoveAnimReferenceSpeed, 0.65f, 1.35f));
	}
	else
	{
		MeshComp->SetPlayRate(1.f);
	}
}

void AMultiGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bCharacterVisualsApplied)
	{
		UpdateLocomotionAnimation();
	}
}

void AMultiGameCharacter::UpdatePlaceholderVisibility()
{
	if (!PlaceholderMesh)
	{
		return;
	}

	const bool bUsePlaceholder = !GetMesh() || GetMesh()->GetSkeletalMeshAsset() == nullptr;
	PlaceholderMesh->SetVisibility(bUsePlaceholder, true);

	if (bUsePlaceholder && PlaceholderMesh->GetStaticMesh())
	{
		if (UMaterialInstanceDynamic* MID = PlaceholderMesh->CreateAndSetMaterialInstanceDynamic(0))
		{
			MID->SetVectorParameterValue(TEXT("Color"), PlaceholderColor);
			MID->SetVectorParameterValue(TEXT("BaseColor"), PlaceholderColor);
			MID->SetVectorParameterValue(TEXT("EmissiveColor"), PlaceholderColor * 3.f);
		}
	}
}

void AMultiGameCharacter::BeginPlay()
{
	Super::BeginPlay();

	EnsureCharacterVisualsLoaded();

	if (HealthComponent)
	{
		HealthComponent->OnDeath.AddDynamic(this, &AMultiGameCharacter::HandleDeath);
	}

	if (HasAuthority() && HealthComponent && Team == ETeam::Enemies)
	{
		HealthComponent->InitMaxHealth(120.f);
	}

	// Placeholder visibility handled in EnsureCharacterVisualsLoaded / UpdatePlaceholderVisibility.
}

bool AMultiGameCharacter::IsDead() const
{
	return HealthComponent && HealthComponent->IsDead();
}

bool AMultiGameCharacter::IsHostileTo(const AMultiGameCharacter* Other) const
{
	return Other && Other->GetTeam() != Team;
}

FVector AMultiGameCharacter::GetAbilityMuzzleLocation() const
{
	if (const USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (MeshComp->DoesSocketExist(TEXT("hand_r")))
		{
			return MeshComp->GetSocketLocation(TEXT("hand_r"));
		}
	}
	return GetActorLocation() + FVector(0.f, 0.f, 60.f);
}

FVector AMultiGameCharacter::GetAbilityAimDirection(const FVector& TargetLocation) const
{
	FVector AimPoint = TargetLocation;
	AimPoint.Z = GetActorLocation().Z + 60.f;

	FVector Direction = (AimPoint - GetAbilityMuzzleLocation()).GetSafeNormal();
	if (Direction.IsNearlyZero())
	{
		Direction = GetActorForwardVector();
	}
	return Direction;
}

void AMultiGameCharacter::MulticastSpawnMeleeFlash_Implementation(FVector Location, float Radius)
{
	FCombatVisuals::SpawnGlowSphere(GetWorld(), Location, Radius, FLinearColor(1.f, 0.2f, 0.05f), 0.22f);
}

void AMultiGameCharacter::MulticastSpawnProjectileVisual_Implementation(FVector SpawnLocation, FVector FlyDirection, float InDamage)
{
	if (HasAuthority() || !GetWorld())
	{
		return;
	}

	const FVector Dir = FlyDirection.GetSafeNormal();
	const FTransform SpawnTransform(Dir.Rotation(), SpawnLocation);

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.Owner = const_cast<AMultiGameCharacter*>(this);
	Params.Instigator = const_cast<AMultiGameCharacter*>(this);

	if (AProjectileBase* Visual = GetWorld()->SpawnActor<AProjectileBase>(AProjectileBase::StaticClass(), SpawnTransform, Params))
	{
		Visual->InitProjectile(InDamage, const_cast<AMultiGameCharacter*>(this), Dir);
		Visual->SetVisualOnly(true);
	}
}

void AMultiGameCharacter::MulticastPlayMontage_Implementation(UAnimMontage* Montage, FName SectionName)
{
	if (!Montage)
	{
		return;
	}

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		if (UAnimInstance* AnimInstance = MeshComp->GetAnimInstance())
		{
			AnimInstance->Montage_Play(Montage);
			if (SectionName != NAME_None)
			{
				AnimInstance->Montage_JumpToSection(SectionName, Montage);
			}
		}
	}
}

void AMultiGameCharacter::MulticastPlayVisualAnimation_Implementation(UAnimSequence* Animation, bool bLooping)
{
	if (!Animation)
	{
		return;
	}

	bLocomotionPaused = true;

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetAnimationMode(EAnimationMode::AnimationSingleNode);
		MeshComp->PlayAnimation(Animation, bLooping);
		MeshComp->SetPlayRate(1.f);
	}

	if (!bLooping)
	{
		CurrentPlayingAnimation = Animation;
	}
}

void AMultiGameCharacter::MulticastResumeIdleAnimation_Implementation()
{
	ResumeLocomotionAnimation();
}

void AMultiGameCharacter::HandleDeath(AActor* DeadActor, AActor* Killer)
{
	OnCharacterDied.Broadcast(this);
	OnDeathEffects();
}

void AMultiGameCharacter::OnDeathEffects()
{
	// Disable collision & movement; enable ragdoll on the mesh everywhere.
	if (UCapsuleComponent* Capsule = GetCapsuleComponent())
	{
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->DisableMovement();
	}

	PrimaryActorTick.SetTickFunctionEnable(false);

	if (USkeletalMeshComponent* MeshComp = GetMesh())
	{
		MeshComp->SetCollisionProfileName(TEXT("Ragdoll"));
		MeshComp->SetSimulatePhysics(true);
	}
}
