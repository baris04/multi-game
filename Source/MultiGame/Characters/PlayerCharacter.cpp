#include "Characters/PlayerCharacter.h"
#include "Characters/MultiGameMannequinSetup.h"
#include "Components/CombatComponent.h"
#include "Components/AbilityComponent.h"
#include "Components/HealthComponent.h"
#include "Core/MultiGamePlayerState.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/SkeletalMesh.h"
#include "Engine/LocalPlayer.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputActionValue.h"
#include "InputModifiers.h"
#include "InputTriggers.h"
#include "GameFramework/InputSettings.h"
#include "Materials/Material.h"
#include "TimerManager.h"

APlayerCharacter::APlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Team = ETeam::Players;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* Move = GetCharacterMovement())
	{
		Move->bOrientRotationToMovement = true;
		Move->RotationRate = FRotator(0.f, 500.f, 0.f);
		Move->MaxWalkSpeed = WalkSpeed;
		Move->JumpZVelocity = 550.f;
		Move->AirControl = 0.35f;
	}

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	HeadMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("HeadMesh"));
	HeadMesh->SetupAttachment(GetMesh());
	HeadMesh->SetLeaderPoseComponent(GetMesh());
	HeadMesh->SetHiddenInGame(true);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	AbilityComponent = CreateDefaultSubobject<UAbilityComponent>(TEXT("AbilityComponent"));
	CombatComponent->SetDamageValues(35.f, 65.f);

	MultiGameMannequin::ApplyVisualPreset(this, MultiGameMannequin::EVisualPreset::Player);

	TArray<FString> MeshPaths;
	TArray<FString> IdleAnimPaths;
	TArray<FString> MoveAnimPaths;
	TArray<FString> MaterialPaths;
	MultiGameMannequin::GetVisualPresetPaths(MultiGameMannequin::EVisualPreset::Player, MeshPaths, IdleAnimPaths, MoveAnimPaths, MaterialPaths);
	if (USkeletalMesh* GideonMesh = MultiGameMannequin::LoadFirstMesh(MeshPaths))
	{
		BodyMeshOptions.Add(GideonMesh);
	}
	MultiGameMannequin::GetVisualPresetPaths(MultiGameMannequin::EVisualPreset::MeleeEnemy, MeshPaths, IdleAnimPaths, MoveAnimPaths, MaterialPaths);
	if (USkeletalMesh* SkeletonMesh = MultiGameMannequin::LoadFirstMesh(MeshPaths))
	{
		BodyMeshOptions.Add(SkeletonMesh);
	}

	// Build Enhanced Input entirely in C++ (no .uasset needed).
	BuildDefaultInput();
}

void APlayerCharacter::BuildDefaultInput()
{
	// Create Input Actions as default sub-objects so they exist without assets.
	MoveAction			= CreateDefaultSubobject<UInputAction>(TEXT("IA_Move"));
	LookAction			= CreateDefaultSubobject<UInputAction>(TEXT("IA_Look"));
	JumpAction			= CreateDefaultSubobject<UInputAction>(TEXT("IA_Jump"));
	SprintAction		= CreateDefaultSubobject<UInputAction>(TEXT("IA_Sprint"));
	DodgeAction			= CreateDefaultSubobject<UInputAction>(TEXT("IA_Dodge"));
	LightAttackAction	= CreateDefaultSubobject<UInputAction>(TEXT("IA_LightAttack"));
	HeavyAttackAction	= CreateDefaultSubobject<UInputAction>(TEXT("IA_HeavyAttack"));
	Ability1Action		= CreateDefaultSubobject<UInputAction>(TEXT("IA_Ability1"));
	Ability2Action		= CreateDefaultSubobject<UInputAction>(TEXT("IA_Ability2"));

	MoveAction->ValueType = EInputActionValueType::Axis2D;
	LookAction->ValueType = EInputActionValueType::Axis2D;
	JumpAction->ValueType = EInputActionValueType::Boolean;
	SprintAction->ValueType = EInputActionValueType::Boolean;
	DodgeAction->ValueType = EInputActionValueType::Boolean;
	LightAttackAction->ValueType = EInputActionValueType::Boolean;
	HeavyAttackAction->ValueType = EInputActionValueType::Boolean;
	Ability1Action->ValueType = EInputActionValueType::Boolean;
	Ability2Action->ValueType = EInputActionValueType::Boolean;

	DefaultMappingContext = CreateDefaultSubobject<UInputMappingContext>(TEXT("IMC_Player"));

	// --- Movement (WASD) with swizzle/negate so one Axis2D covers all 4 keys. ---
	// W -> +Y, S -> -Y, D -> +X, A -> -X (X = right, Y = forward in Input_Move).
	// Modifiers must use CreateDefaultSubobject in the constructor (NewObject crashes).
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::W);
		M.Modifiers.Add(CreateDefaultSubobject<UInputModifierSwizzleAxis>(TEXT("MoveMod_SwizzleW")));
	}
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::S);
		M.Modifiers.Add(CreateDefaultSubobject<UInputModifierNegate>(TEXT("MoveMod_NegateS")));
		M.Modifiers.Add(CreateDefaultSubobject<UInputModifierSwizzleAxis>(TEXT("MoveMod_SwizzleS")));
	}
	{
		DefaultMappingContext->MapKey(MoveAction, EKeys::D);
	}
	{
		FEnhancedActionKeyMapping& M = DefaultMappingContext->MapKey(MoveAction, EKeys::A);
		M.Modifiers.Add(CreateDefaultSubobject<UInputModifierNegate>(TEXT("MoveMod_NegateA")));
	}

	// --- Look (mouse). Negate Y so moving mouse up looks up. ---
	{
		DefaultMappingContext->MapKey(LookAction, EKeys::Mouse2D);
	}

	// --- Buttons ---
	DefaultMappingContext->MapKey(JumpAction, EKeys::SpaceBar);
	DefaultMappingContext->MapKey(SprintAction, EKeys::LeftShift);
	DefaultMappingContext->MapKey(DodgeAction, EKeys::LeftControl);
	DefaultMappingContext->MapKey(LightAttackAction, EKeys::LeftMouseButton);
	DefaultMappingContext->MapKey(HeavyAttackAction, EKeys::RightMouseButton);
	DefaultMappingContext->MapKey(Ability1Action, EKeys::Q);
	DefaultMappingContext->MapKey(Ability2Action, EKeys::E);
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && HealthComponent)
	{
		HealthComponent->InitMaxHealth(500.f);
		HealthComponent->SetIncomingDamageMultiplier(0.55f);
		HealthComponent->SetDamageReceiveCooldown(0.4f);
	}

	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}

	RefreshAppearanceFromPlayerState();
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Appearance may replicate after the pawn spawns; apply once available.
	if (!bAppearanceApplied)
	{
		RefreshAppearanceFromPlayerState();
	}
}

void APlayerCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	RefreshAppearanceFromPlayerState();
	EnsureInputContext();
}

void APlayerCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	RefreshAppearanceFromPlayerState();
}

void APlayerCharacter::RefreshAppearanceFromPlayerState()
{
	if (AMultiGamePlayerState* PS = GetPlayerState<AMultiGamePlayerState>())
	{
		ApplyAppearance(PS->GetAppearance());
	}
}

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	EnsureInputContext();

	UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (!EIC)
	{
		return;
	}

	if (MoveAction)		 { EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Move); }
	if (LookAction)		 { EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::Input_Look); }
	if (JumpAction)
	{
		EIC->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
	}
	if (SprintAction)
	{
		EIC->BindAction(SprintAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_SprintStart);
		EIC->BindAction(SprintAction, ETriggerEvent::Completed, this, &APlayerCharacter::Input_SprintStop);
	}
	if (DodgeAction)		{ EIC->BindAction(DodgeAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_Dodge); }
	if (LightAttackAction)	{ EIC->BindAction(LightAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_LightAttack); }
	if (HeavyAttackAction)	{ EIC->BindAction(HeavyAttackAction, ETriggerEvent::Started, this, &APlayerCharacter::Input_HeavyAttack); }
	if (Ability1Action)		{ EIC->BindAction(Ability1Action, ETriggerEvent::Started, this, &APlayerCharacter::Input_Ability1); }
	if (Ability2Action)		{ EIC->BindAction(Ability2Action, ETriggerEvent::Started, this, &APlayerCharacter::Input_Ability2); }
}

void APlayerCharacter::Input_Move(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	const FVector2D Axis = Value.Get<FVector2D>();
	if (Controller && !Axis.IsNearlyZero())
	{
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		const FVector Forward = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector Right = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Forward, Axis.Y);
		AddMovementInput(Right, Axis.X);
	}
}

void APlayerCharacter::Input_Look(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	AddControllerYawInput(Axis.X);
	// Negate so moving the mouse up looks up (raw mouse Y is inverted).
	AddControllerPitchInput(-Axis.Y);
}

void APlayerCharacter::Input_SprintStart(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	bIsSprinting = true;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed; // local responsiveness
	}
	ServerSetSprint(true);
}

void APlayerCharacter::Input_SprintStop(const FInputActionValue& Value)
{
	bIsSprinting = false;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	ServerSetSprint(false);
}

void APlayerCharacter::ServerSetSprint_Implementation(bool bNewSprinting)
{
	bIsSprinting = bNewSprinting;
	if (GetCharacterMovement())
	{
		GetCharacterMovement()->MaxWalkSpeed = bNewSprinting ? SprintSpeed : WalkSpeed;
	}
}

void APlayerCharacter::Input_Dodge(const FInputActionValue& Value)
{
	if (IsDead() || bIsDodging)
	{
		return;
	}

	FVector DodgeDir = GetLastMovementInputVector();
	if (DodgeDir.IsNearlyZero())
	{
		DodgeDir = GetActorForwardVector();
	}
	DodgeDir = DodgeDir.GetSafeNormal();

	ServerDodge(DodgeDir);
}

void APlayerCharacter::ServerDodge_Implementation(FVector DodgeDirection)
{
	if (IsDead() || bIsDodging)
	{
		return;
	}

	bIsDodging = true;

	if (HealthComponent)
	{
		HealthComponent->SetInvulnerable(true);
	}

	GetWorldTimerManager().SetTimer(DodgeIFrameTimer, this, &APlayerCharacter::EndDodgeInvulnerability, DodgeInvulnerabilitySeconds, false);

	MulticastDodge(DodgeDirection);
}

void APlayerCharacter::MulticastDodge_Implementation(FVector DodgeDirection)
{
	if (DodgeMontage)
	{
		if (UAnimInstance* Anim = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
		{
			Anim->Montage_Play(DodgeMontage);
		}
	}

	LaunchCharacter(DodgeDirection * DodgeImpulse, true, false);
}

void APlayerCharacter::EndDodgeInvulnerability()
{
	bIsDodging = false;
	if (HealthComponent)
	{
		HealthComponent->SetInvulnerable(false);
	}
}

void APlayerCharacter::EnsureInputContext()
{
	if (!IsLocallyControlled() || !DefaultMappingContext)
	{
		return;
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void APlayerCharacter::Input_LightAttack(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	if (HasAuthority())
	{
		if (CombatComponent)
		{
			CombatComponent->RequestLightAttack();
		}
	}
	else
	{
		ServerLightAttack();
	}
}

void APlayerCharacter::ServerLightAttack_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->RequestLightAttack();
	}
}

void APlayerCharacter::Input_HeavyAttack(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	if (HasAuthority())
	{
		if (CombatComponent)
		{
			CombatComponent->RequestHeavyAttack();
		}
	}
	else
	{
		ServerHeavyAttack();
	}
}

void APlayerCharacter::ServerHeavyAttack_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->RequestHeavyAttack();
	}
}

void APlayerCharacter::Input_Ability1(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	const FVector Target = GetAbilityTargetLocation();
	if (HasAuthority())
	{
		if (AbilityComponent)
		{
			AbilityComponent->TryCastAbility(0, Target);
		}
	}
	else
	{
		ServerCastAbility(0, Target);
	}
}

void APlayerCharacter::Input_Ability2(const FInputActionValue& Value)
{
	if (IsDead())
	{
		return;
	}

	const FVector Target = GetAbilityTargetLocation();
	if (HasAuthority())
	{
		if (AbilityComponent)
		{
			AbilityComponent->TryCastAbility(1, Target);
		}
	}
	else
	{
		ServerCastAbility(1, Target);
	}
}

void APlayerCharacter::ServerCastAbility_Implementation(int32 AbilityIndex, FVector TargetLocation)
{
	if (AbilityComponent)
	{
		AbilityComponent->TryCastAbility(AbilityIndex, TargetLocation);
	}
}

FVector APlayerCharacter::GetAbilityTargetLocation() const
{
	FVector Origin = GetActorLocation() + FVector(0.f, 0.f, 60.f);
	FVector Dir = GetActorForwardVector();

	if (FollowCamera)
	{
		Origin = FollowCamera->GetComponentLocation();
		Dir = FollowCamera->GetForwardVector();
	}
	else if (Controller)
	{
		const FRotator YawRot(0.f, Controller->GetControlRotation().Yaw, 0.f);
		Dir = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
	}

	Dir.Z = FMath::Clamp(Dir.Z, -0.2f, 0.25f);
	Dir.Normalize();

	return Origin + Dir * 2500.f;
}

FVector APlayerCharacter::GetAbilityMuzzleLocation() const
{
	if (FollowCamera)
	{
		return FollowCamera->GetComponentLocation() + FollowCamera->GetForwardVector() * 80.f;
	}
	return Super::GetAbilityMuzzleLocation();
}

FVector APlayerCharacter::GetAbilityAimDirection(const FVector& TargetLocation) const
{
	FVector Dir = (TargetLocation - GetAbilityMuzzleLocation()).GetSafeNormal();
	if (Dir.IsNearlyZero() && FollowCamera)
	{
		Dir = FollowCamera->GetForwardVector();
		Dir.Z = FMath::Clamp(Dir.Z, -0.2f, 0.25f);
		Dir.Normalize();
	}
	return Dir;
}

void APlayerCharacter::ApplyAppearance(const FCharacterAppearance& Appearance)
{
	USkeletalMeshComponent* BodyMeshComp = GetMesh();
	if (!BodyMeshComp)
	{
		return;
	}

	bool bMeshChanged = false;
	if (BodyMeshOptions.IsValidIndex(Appearance.BodyMeshIndex) && BodyMeshOptions[Appearance.BodyMeshIndex])
	{
		if (BodyMeshComp->GetSkeletalMeshAsset() != BodyMeshOptions[Appearance.BodyMeshIndex])
		{
			BodyMeshComp->SetSkeletalMesh(BodyMeshOptions[Appearance.BodyMeshIndex]);
			bMeshChanged = true;
		}
	}

	if (HeadMesh && HeadMeshOptions.IsValidIndex(Appearance.HeadMeshIndex) && HeadMeshOptions[Appearance.HeadMeshIndex])
	{
		HeadMesh->SetSkeletalMesh(HeadMeshOptions[Appearance.HeadMeshIndex]);
		HeadMesh->SetLeaderPoseComponent(BodyMeshComp);
	}

	// Only tint when the character creation flow set a custom primary colour; otherwise keep
	// the Paragon model's own materials so it doesn't turn into a solid colour blob.
	if (!Appearance.PrimaryColor.Equals(FLinearColor::White))
	{
		const int32 NumMats = BodyMeshComp->GetNumMaterials();
		for (int32 i = 0; i < NumMats; ++i)
		{
			if (UMaterialInstanceDynamic* MID = BodyMeshComp->CreateAndSetMaterialInstanceDynamic(i))
			{
				MID->SetVectorParameterValue(PrimaryColorParam, Appearance.PrimaryColor);
				MID->SetVectorParameterValue(SecondaryColorParam, Appearance.SecondaryColor);
			}
		}
	}

	// SetSkeletalMesh resets the pose to reference (T-pose); restart the idle loop.
	if (bMeshChanged)
	{
		ResumeLocomotionAnimation();
	}

	bAppearanceApplied = true;
}

void APlayerCharacter::OnDeathEffects()
{
	Super::OnDeathEffects();

	// Stop input on the owning client / server for a downed player.
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		DisableInput(PC);
	}
}
