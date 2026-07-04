#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;
class UPointLightComponent;
class UDamageType;

/**
 * Replicated projectile spawned by the ability system (player mages & caster enemies).
 * Movement is replicated; damage is applied only on the server.
 */
UCLASS()
class MULTIGAME_API AProjectileBase : public AActor
{
	GENERATED_BODY()

public:
	AProjectileBase();

	/** Set by the spawner so we don't damage our own instigator and can scale damage. */
	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void InitProjectile(float InDamage, AActor* InShooter, const FVector& FlyDirection);

	/** Visual-only copy for clients (no damage logic). */
	void SetVisualOnly(bool bInVisualOnly);

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	void TryDamageActor(AActor* OtherActor);

	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& Hit);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<USphereComponent> CollisionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UStaticMeshComponent> MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Projectile")
	TObjectPtr<UPointLightComponent> GlowLight;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float Damage = 20.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float InitialSpeed = 2000.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float LifeSpanSeconds = 5.f;

	/** Optional radial explosion on impact (0 = single-target). */
	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	float ImpactRadius = 0.f;

	UPROPERTY(EditDefaultsOnly, Category = "Projectile")
	TSubclassOf<UDamageType> DamageTypeClass;

	UPROPERTY()
	TObjectPtr<AActor> Shooter;

	bool bVisualOnly = false;
	bool bCanDealDamage = false;
	float SpawnWorldTime = 0.f;
};
