#include "Components/CombatComponent.h"
#include "Characters/MultiGameCharacter.h"
#include "Components/HealthComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Controller.h"
#include "Animation/AnimSequence.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	DamageTypeClass = UDamageType::StaticClass();
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	OwnerCharacter = Cast<AMultiGameCharacter>(GetOwner());
	LoadDefaultAttackAnimations();
}

void UCombatComponent::SetDamageValues(float Light, float Heavy)
{
	LightDamage = Light;
	HeavyDamage = Heavy;
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDamageWindowOpen)
	{
		PerformTrace();
	}
}

void UCombatComponent::RequestLightAttack()
{
	if (!OwnerCharacter || OwnerCharacter->IsDead())
	{
		return;
	}

	if (LightComboMontages.Num() == 0)
	{
		if (LightAttackAnimations.Num() > 0)
		{
			const double Now = FPlatformTime::Seconds();
			if (bIsAttacking && (Now - LastAttackTime) < ComboWindowSeconds)
			{
				ComboIndex = (ComboIndex + 1) % LightAttackAnimations.Num();
			}
			else if (!bIsAttacking)
			{
				ComboIndex = 0;
			}
			else
			{
				return;
			}

			PerformAnimatedMelee(LightAttackAnimations[ComboIndex], LightDamage);
			return;
		}

		PerformInstantMelee(LightDamage);
		return;
	}

	const double Now = FPlatformTime::Seconds();

	// If mid-attack outside the combo window, ignore. Otherwise advance the chain.
	if (bIsAttacking && (Now - LastAttackTime) < ComboWindowSeconds)
	{
		ComboIndex = (ComboIndex + 1) % LightComboMontages.Num();
	}
	else if (!bIsAttacking)
	{
		ComboIndex = 0;
	}
	else
	{
		return;
	}

	PerformAttack(LightComboMontages[ComboIndex], NAME_None, LightDamage);
}

void UCombatComponent::RequestHeavyAttack()
{
	if (!OwnerCharacter || OwnerCharacter->IsDead() || bIsAttacking)
	{
		return;
	}

	if (!HeavyAttackMontage)
	{
		if (HeavyAttackAnimation)
		{
			ComboIndex = 0;
			PerformAnimatedMelee(HeavyAttackAnimation, HeavyDamage);
			return;
		}

		PerformInstantMelee(HeavyDamage);
		return;
	}

	ComboIndex = 0;
	PerformAttack(HeavyAttackMontage, NAME_None, HeavyDamage);
}

void UCombatComponent::PerformAttack(UAnimMontage* Montage, FName Section, float InDamage)
{
	if (!Montage || !OwnerCharacter)
	{
		PerformInstantMelee(InDamage);
		return;
	}

	bIsAttacking = true;
	CurrentDamage = InDamage;
	LastAttackTime = FPlatformTime::Seconds();

	OwnerCharacter->MulticastPlayMontage(Montage, Section);

	// Fallback reset in case a notify fails to close the window / montage is interrupted.
	const float ResetDelay = Montage->GetPlayLength() + 0.1f;
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AttackResetTimer, this, &UCombatComponent::ResetCombo, ResetDelay, false);
	}
}

void UCombatComponent::LoadDefaultAttackAnimations()
{
	if (!OwnerCharacter || OwnerCharacter->GetVisualPreset() != MultiGameMannequin::EVisualPreset::Player)
	{
		return;
	}

	if (LightAttackAnimations.Num() == 0)
	{
		const TCHAR* LightPaths[] = {
			TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Primary_Attack_A_Medium.Primary_Attack_A_Medium"),
			TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Primary_Attack_B_Medium.Primary_Attack_B_Medium")
		};

		for (const TCHAR* Path : LightPaths)
		{
			if (UAnimSequence* Animation = LoadObject<UAnimSequence>(nullptr, Path))
			{
				LightAttackAnimations.Add(Animation);
			}
		}
	}

	if (!HeavyAttackAnimation)
	{
		HeavyAttackAnimation = LoadObject<UAnimSequence>(
			nullptr,
			TEXT("/Game/ParagonGideon/Characters/Heroes/Gideon/Animations/Primary_Attack_C_Medium.Primary_Attack_C_Medium"));
	}
}

void UCombatComponent::PerformAnimatedMelee(UAnimSequence* Animation, float InDamage)
{
	if (!Animation || !OwnerCharacter)
	{
		PerformInstantMelee(InDamage);
		return;
	}

	bIsAttacking = true;
	CurrentDamage = InDamage;
	LastAttackTime = FPlatformTime::Seconds();
	HitActorsThisWindow.Reset();

	OwnerCharacter->MulticastPlayVisualAnimation(Animation, false);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(DamageFallbackTimer);
		World->GetTimerManager().ClearTimer(AttackResetTimer);
		World->GetTimerManager().SetTimer(DamageFallbackTimer, this, &UCombatComponent::PerformMeleeSweep, 0.18f, false);
		World->GetTimerManager().SetTimer(AttackResetTimer, this, &UCombatComponent::ResetCombo, FMath::Max(0.35f, Animation->GetPlayLength()), false);
	}
}

void UCombatComponent::OpenDamageWindow()
{
	bDamageWindowOpen = true;
	HitActorsThisWindow.Reset();
	SetComponentTickEnabled(true);
}

void UCombatComponent::CloseDamageWindow()
{
	bDamageWindowOpen = false;
	SetComponentTickEnabled(false);
	bIsAttacking = false;
}

void UCombatComponent::PerformTrace()
{
	PerformMeleeSweep();
}

FVector UCombatComponent::GetMeleeForward() const
{
	if (!OwnerCharacter)
	{
		return FVector::ForwardVector;
	}

	if (const APawn* Pawn = Cast<APawn>(OwnerCharacter))
	{
		if (const AController* Controller = Pawn->GetController())
		{
			const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
			return FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		}
	}

	return OwnerCharacter->GetActorForwardVector();
}

void UCombatComponent::PerformMeleeSweep()
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !OwnerCharacter)
	{
		return;
	}

	const FVector Forward = GetMeleeForward();
	const FVector Center = OwnerCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f) + Forward * (TraceReach * 0.5f);
	const float Radius = TraceRadius + 80.f;

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjParams(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeSweep), false, OwnerCharacter);

	GetWorld()->OverlapMultiByObjectType(Overlaps, Center, FQuat::Identity, ObjParams, FCollisionShape::MakeSphere(Radius), Params);

	AController* InstigatorController = OwnerCharacter->GetController();

	for (const FOverlapResult& Overlap : Overlaps)
	{
		AActor* HitActor = Overlap.GetActor();
		if (!HitActor || HitActorsThisWindow.Contains(HitActor))
		{
			continue;
		}

		AMultiGameCharacter* HitCharacter = Cast<AMultiGameCharacter>(HitActor);
		if (HitCharacter && OwnerCharacter->IsHostileTo(HitCharacter))
		{
			HitActorsThisWindow.Add(HitActor);
			if (UHealthComponent* Health = HitCharacter->GetHealthComponent())
			{
				Health->ApplyDamageServer(CurrentDamage, OwnerCharacter, InstigatorController);
			}
			else
			{
				UGameplayStatics::ApplyDamage(HitActor, CurrentDamage, InstigatorController, OwnerCharacter, DamageTypeClass);
			}
		}
	}
}

void UCombatComponent::ResetCombo()
{
	bIsAttacking = false;
	bDamageWindowOpen = false;
	ComboIndex = 0;
	SetComponentTickEnabled(false);

	if (OwnerCharacter)
	{
		OwnerCharacter->MulticastResumeIdleAnimation();
	}
}

void UCombatComponent::PerformInstantMelee(float InDamage)
{
	if (!GetOwner() || !GetOwner()->HasAuthority() || !OwnerCharacter || OwnerCharacter->IsDead())
	{
		return;
	}

	CurrentDamage = InDamage;
	bIsAttacking = true;
	LastAttackTime = FPlatformTime::Seconds();

	HitActorsThisWindow.Reset();
	PerformMeleeSweep();

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(AttackResetTimer, this, &UCombatComponent::ResetCombo, 0.25f, false);
	}
}
