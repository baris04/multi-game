#include "Combat/ProjectileBase.h"
#include "Characters/MultiGameCharacter.h"
#include "Combat/CombatVisuals.h"
#include "Components/HealthComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/DamageType.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "UObject/ConstructorHelpers.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(33.f);

	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComp"));
	CollisionComp->InitSphereRadius(32.f);
	CollisionComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionComp->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionComp->SetGenerateOverlapEvents(true);
	RootComponent = CollisionComp;

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	MeshComp->SetupAttachment(RootComponent);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetRelativeScale3D(FVector(1.6f));
	MeshComp->SetCastShadow(false);

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(
		TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (SphereMesh.Succeeded())
	{
		MeshComp->SetStaticMesh(SphereMesh.Object);
	}

	static ConstructorHelpers::FObjectFinder<UMaterial> BasicMat(
		TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	if (BasicMat.Succeeded())
	{
		MeshComp->SetMaterial(0, BasicMat.Object);
	}

	GlowLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("GlowLight"));
	GlowLight->SetupAttachment(RootComponent);
	GlowLight->SetIntensity(12000.f);
	GlowLight->SetAttenuationRadius(300.f);
	GlowLight->SetLightColor(FLinearColor(1.f, 0.45f, 0.05f));
	GlowLight->SetCastShadows(false);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->ProjectileGravityScale = 0.f;
	ProjectileMovement->bInitialVelocityInLocalSpace = false;

	DamageTypeClass = UDamageType::StaticClass();
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	SpawnWorldTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;

	ProjectileMovement->InitialSpeed = InitialSpeed;
	ProjectileMovement->MaxSpeed = InitialSpeed;
	SetLifeSpan(LifeSpanSeconds);
	SetActorHiddenInGame(false);

	if (MeshComp)
	{
		MeshComp->SetVisibility(true, true);
		FCombatVisuals::ApplyGlowMaterial(MeshComp, FLinearColor(1.f, 0.45f, 0.05f), 10.f);
	}

	if (bVisualOnly || !HasAuthority())
	{
		SetActorTickEnabled(false);
	}
	else
	{
		CollisionComp->OnComponentBeginOverlap.AddDynamic(this, &AProjectileBase::OnOverlap);
	}
}

void AProjectileBase::SetVisualOnly(bool bInVisualOnly)
{
	bVisualOnly = bInVisualOnly;
	bCanDealDamage = false;
	SetActorTickEnabled(false);
}

void AProjectileBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || bVisualOnly || !bCanDealDamage)
	{
		return;
	}

	if (GetWorld() && (GetWorld()->GetTimeSeconds() - SpawnWorldTime) < 0.12f)
	{
		return;
	}

	TArray<FOverlapResult> Overlaps;
	FCollisionObjectQueryParams ObjParams(ECC_Pawn);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(ProjectileHit), false, this);
	if (Shooter)
	{
		Params.AddIgnoredActor(Shooter);
	}

	GetWorld()->OverlapMultiByObjectType(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ObjParams,
		FCollisionShape::MakeSphere(CollisionComp->GetScaledSphereRadius() + 16.f),
		Params);

	for (const FOverlapResult& Overlap : Overlaps)
	{
		TryDamageActor(Overlap.GetActor());
	}
}

void AProjectileBase::InitProjectile(float InDamage, AActor* InShooter, const FVector& FlyDirection)
{
	Damage = InDamage;
	Shooter = InShooter;
	bCanDealDamage = HasAuthority() && !bVisualOnly;

	if (InShooter)
	{
		CollisionComp->IgnoreActorWhenMoving(InShooter, true);
	}

	const FVector Dir = FlyDirection.GetSafeNormal();
	if (!Dir.IsNearlyZero())
	{
		SetActorRotation(Dir.Rotation());
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = Dir * InitialSpeed;
		ProjectileMovement->UpdateComponentVelocity();
		ProjectileMovement->Activate(true);
	}
}

void AProjectileBase::TryDamageActor(AActor* OtherActor)
{
	if (!HasAuthority() || bVisualOnly || !bCanDealDamage || !OtherActor || OtherActor == this || OtherActor == Shooter)
	{
		return;
	}

	AMultiGameCharacter* HitCharacter = Cast<AMultiGameCharacter>(OtherActor);
	if (!HitCharacter)
	{
		return;
	}

	if (const AMultiGameCharacter* ShooterChar = Cast<AMultiGameCharacter>(Shooter))
	{
		if (!ShooterChar->IsHostileTo(HitCharacter))
		{
			return;
		}
	}

	AController* InstigatorController = Shooter ? Shooter->GetInstigatorController() : nullptr;

	if (ImpactRadius > 0.f)
	{
		UGameplayStatics::ApplyRadialDamage(this, Damage, GetActorLocation(), ImpactRadius,
			DamageTypeClass, TArray<AActor*>{ Shooter }, this, InstigatorController, true);
	}
	else if (UHealthComponent* Health = HitCharacter->GetHealthComponent())
	{
		Health->ApplyDamageServer(Damage, Shooter, InstigatorController);
	}
	else
	{
		UGameplayStatics::ApplyDamage(OtherActor, Damage, InstigatorController, this, DamageTypeClass);
	}

	Destroy();
}

void AProjectileBase::OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit)
{
	TryDamageActor(OtherActor);
}
