#include "Combat/CombatVisuals.h"
#include "Components/MeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"

void FCombatVisuals::ApplyGlowMaterial(UMeshComponent* MeshComp, const FLinearColor& Color, float EmissiveScale)
{
	if (!MeshComp)
	{
		return;
	}

	if (!MeshComp->GetMaterial(0))
	{
		if (UMaterial* BaseMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
		{
			MeshComp->SetMaterial(0, BaseMat);
		}
	}

	if (UMaterialInstanceDynamic* MID = MeshComp->CreateAndSetMaterialInstanceDynamic(0))
	{
		MID->SetVectorParameterValue(TEXT("Color"), Color);
		MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
		MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color * EmissiveScale);
	}
}

void FCombatVisuals::SpawnGlowSphere(UWorld* World, const FVector& Location, float Radius, const FLinearColor& Color, float LifeSeconds)
{
	if (!World)
	{
		return;
	}

	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AStaticMeshActor* FX = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, FRotator::ZeroRotator, Params);
	if (!FX)
	{
		return;
	}

	UStaticMesh* SphereMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	if (UStaticMeshComponent* Mesh = FX->GetStaticMeshComponent())
	{
		Mesh->SetMobility(EComponentMobility::Movable);
		Mesh->SetStaticMesh(SphereMesh);
		Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		Mesh->SetCastShadow(false);

		const float Scale = FMath::Max(0.15f, (Radius * 2.f) / 100.f);
		Mesh->SetWorldScale3D(FVector(Scale));

		ApplyGlowMaterial(Mesh, Color, 8.f);
	}

	UPointLightComponent* Light = NewObject<UPointLightComponent>(FX, TEXT("FlashLight"));
	Light->SetupAttachment(FX->GetRootComponent());
	Light->SetIntensity(10000.f);
	Light->SetAttenuationRadius(Radius * 4.f);
	Light->SetLightColor(Color);
	Light->SetCastShadows(false);
	Light->RegisterComponent();

	FX->SetLifeSpan(LifeSeconds);
}
