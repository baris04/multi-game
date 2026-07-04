#include "Core/MultiGameSceneSetup.h"
#include "Engine/World.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/SkyAtmosphereComponent.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "Kismet/GameplayStatics.h"

namespace
{
	const float SunIntensityLux = 25.f;
	const float SkyLightIntensity = 5.f;
	const float FogDensity = 0.018f;

	void BoostOrSpawnDirectionalLight(UWorld* World)
	{
		TArray<AActor*> DirLights;
		UGameplayStatics::GetAllActorsOfClass(World, ADirectionalLight::StaticClass(), DirLights);

		if (DirLights.Num() == 0)
		{
			DirLights.Add(World->SpawnActor<ADirectionalLight>(
				ADirectionalLight::StaticClass(), FVector(0.f, 0.f, 1200.f), FRotator(-48.f, -35.f, 0.f)));
		}

		for (AActor* Actor : DirLights)
		{
			ADirectionalLight* Sun = Cast<ADirectionalLight>(Actor);
			if (!Sun)
			{
				continue;
			}

			if (UDirectionalLightComponent* Light = Sun->GetComponent())
			{
				Light->SetMobility(EComponentMobility::Movable);
				Light->SetIntensity(SunIntensityLux);
				Light->SetLightColor(FLinearColor(1.f, 0.97f, 0.9f));
				Light->SetTemperature(5800.f);
				Light->bEnableLightShaftOcclusion = false;
			}
		}
	}

	void BoostOrSpawnSkyLight(UWorld* World)
	{
		TArray<AActor*> SkyLights;
		UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);

		if (SkyLights.Num() == 0)
		{
			SkyLights.Add(World->SpawnActor<ASkyLight>(
				ASkyLight::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator));
		}

		for (AActor* Actor : SkyLights)
		{
			ASkyLight* Sky = Cast<ASkyLight>(Actor);
			if (!Sky)
			{
				continue;
			}

			if (USkyLightComponent* Light = Sky->GetLightComponent())
			{
				Light->SetMobility(EComponentMobility::Movable);
				Light->bRealTimeCapture = true;
				Light->SetIntensity(SkyLightIntensity);
				Light->SetLightColor(FLinearColor(0.75f, 0.85f, 1.f));
				Light->RecaptureSky();
			}
		}
	}

	void EnsureSkyAtmosphere(UWorld* World)
	{
		TArray<AActor*> Atmospheres;
		UGameplayStatics::GetAllActorsOfClass(World, ASkyAtmosphere::StaticClass(), Atmospheres);
		if (Atmospheres.Num() > 0)
		{
			return;
		}

		if (ASkyAtmosphere* Atmosphere = World->SpawnActor<ASkyAtmosphere>(
			ASkyAtmosphere::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
		{
			if (USkyAtmosphereComponent* AtmoComp = Atmosphere->GetComponent())
			{
				AtmoComp->SetMobility(EComponentMobility::Movable);
			}
		}
	}

	void EnsureHeightFog(UWorld* World)
	{
		TArray<AActor*> Fogs;
		UGameplayStatics::GetAllActorsOfClass(World, AExponentialHeightFog::StaticClass(), Fogs);
		if (Fogs.Num() > 0)
		{
			for (AActor* Actor : Fogs)
			{
				if (AExponentialHeightFog* Fog = Cast<AExponentialHeightFog>(Actor))
				{
					if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
					{
						FogComp->SetFogDensity(FMath::Max(FogComp->FogDensity, FogDensity));
						FogComp->SetFogInscatteringColor(FLinearColor(0.55f, 0.65f, 0.85f));
					}
				}
			}
			return;
		}

		if (AExponentialHeightFog* Fog = World->SpawnActor<AExponentialHeightFog>(
			AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
		{
			if (UExponentialHeightFogComponent* FogComp = Fog->GetComponent())
			{
				FogComp->SetFogDensity(FogDensity);
				FogComp->SetFogHeightFalloff(0.12f);
				FogComp->SetFogInscatteringColor(FLinearColor(0.55f, 0.65f, 0.85f));
				FogComp->SetDirectionalInscatteringColor(FLinearColor(0.9f, 0.85f, 0.7f));
				FogComp->SetDirectionalInscatteringExponent(5.f);
				FogComp->SetStartDistance(0.f);
			}
		}
	}

	void EnsurePostProcess(UWorld* World)
	{
		TArray<AActor*> Volumes;
		UGameplayStatics::GetAllActorsOfClass(World, APostProcessVolume::StaticClass(), Volumes);

		APostProcessVolume* Volume = nullptr;
		for (AActor* Actor : Volumes)
		{
			if (APostProcessVolume* PP = Cast<APostProcessVolume>(Actor))
			{
				if (PP->bUnbound)
				{
					Volume = PP;
					break;
				}
			}
		}

		if (!Volume)
		{
			Volume = World->SpawnActor<APostProcessVolume>(
				APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		}

		if (!Volume)
		{
			return;
		}

		Volume->bUnbound = true;

		FPostProcessSettings& PP = Volume->Settings;
		PP.bOverride_AutoExposureBias = true;
		PP.AutoExposureBias = 1.75f;
		PP.bOverride_AutoExposureMinBrightness = true;
		PP.AutoExposureMinBrightness = 0.8f;
		PP.bOverride_AutoExposureMaxBrightness = true;
		PP.AutoExposureMaxBrightness = 3.5f;
		PP.bOverride_ColorContrast = true;
		PP.ColorContrast = FVector4(1.05f, 1.05f, 1.05f, 1.f);
		PP.bOverride_ColorGamma = true;
		PP.ColorGamma = FVector4(1.05f, 1.05f, 1.05f, 1.f);
	}

	AStaticMeshActor* SpawnDecoActor(
		UWorld* World,
		UStaticMesh* Mesh,
		const FVector& Location,
		const FRotator& Rotation,
		const FVector& Scale,
		const FLinearColor& Color,
		float EmissiveScale = 0.4f,
		bool bEnableCollision = false)
	{
		if (!World || !Mesh)
		{
			return nullptr;
		}

		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AStaticMeshActor* Actor = World->SpawnActor<AStaticMeshActor>(
			AStaticMeshActor::StaticClass(), Location, Rotation, Params);
		if (!Actor)
		{
			return nullptr;
		}

		Actor->SetMobility(EComponentMobility::Static);
		if (UStaticMeshComponent* SMC = Actor->GetStaticMeshComponent())
		{
			SMC->SetStaticMesh(Mesh);
			SMC->SetWorldScale3D(Scale);
			SMC->SetMobility(EComponentMobility::Static);
			SMC->SetCollisionEnabled(bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision);
			SMC->SetCastShadow(true);

			if (UMaterial* BaseMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
			{
				SMC->SetMaterial(0, BaseMat);
				if (UMaterialInstanceDynamic* MID = SMC->CreateAndSetMaterialInstanceDynamic(0))
				{
					MID->SetVectorParameterValue(TEXT("Color"), Color);
					MID->SetVectorParameterValue(TEXT("BaseColor"), Color);
					MID->SetVectorParameterValue(TEXT("EmissiveColor"), Color * EmissiveScale);
				}
			}
		}

		return Actor;
	}

	void AttachTorchLight(AStaticMeshActor* TorchBase, const FLinearColor& LightColor, float Intensity = 8000.f)
	{
		if (!TorchBase)
		{
			return;
		}

		UPointLightComponent* Light = NewObject<UPointLightComponent>(TorchBase, TEXT("TorchLight"));
		Light->SetupAttachment(TorchBase->GetRootComponent());
		Light->SetRelativeLocation(FVector(0.f, 0.f, 180.f));
		Light->SetIntensity(Intensity);
		Light->SetAttenuationRadius(900.f);
		Light->SetLightColor(LightColor);
		Light->SetCastShadows(false);
		Light->RegisterComponent();
	}
}

void MultiGameSceneSetup::ApplyGameplayLighting(UWorld* World)
{
	if (!World)
	{
		return;
	}

	EnsureSkyAtmosphere(World);
	BoostOrSpawnDirectionalLight(World);
	BoostOrSpawnSkyLight(World);
	EnsureHeightFog(World);
	EnsurePostProcess(World);
}

void MultiGameSceneSetup::ApplyArenaDecoration(UWorld* World, float ArenaHalfSize)
{
	if (!World || ArenaHalfSize <= 0.f)
	{
		return;
	}

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMesh* CylinderMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	UStaticMesh* ConeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cone.Cone"));
	if (!CubeMesh || !CylinderMesh)
	{
		return;
	}

	const FLinearColor StoneColor(0.28f, 0.30f, 0.34f);
	const FLinearColor AccentBlue(0.15f, 0.45f, 0.95f);
	const FLinearColor AccentGold(0.95f, 0.72f, 0.25f);
	const FLinearColor AccentRed(0.85f, 0.22f, 0.15f);
	const FLinearColor PlatformColor(0.22f, 0.24f, 0.30f);

	// --- Center fighting platform (visual only, no collision) ---
	SpawnDecoActor(World, CylinderMesh, FVector(0.f, 0.f, 6.f), FRotator::ZeroRotator,
		FVector(5.f, 5.f, 0.08f), PlatformColor, 0.25f, false);

	SpawnDecoActor(World, CylinderMesh, FVector(0.f, 0.f, 10.f), FRotator::ZeroRotator,
		FVector(3.5f, 3.5f, 0.04f), AccentBlue * 0.6f, 1.2f, false);

	// --- Inner floor ring ---
	const float InnerRingRadius = ArenaHalfSize * 0.35f;
	for (int32 i = 0; i < 16; ++i)
	{
		const float Angle = (360.f / 16.f) * i;
		const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * InnerRingRadius + FVector(0.f, 0.f, 4.f);
		SpawnDecoActor(World, CubeMesh, Loc, FRotator(0.f, Angle, 0.f),
			FVector(0.5f, 0.5f, 0.08f), AccentBlue * 0.5f, 0.8f);
	}

	// --- Outer boundary pillars ---
	const int32 NumPillars = 12;
	const float PillarRadius = ArenaHalfSize * 0.88f;
	for (int32 i = 0; i < NumPillars; ++i)
	{
		const float Angle = (360.f / NumPillars) * i;
		const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * PillarRadius + FVector(0.f, 0.f, 120.f);

		if (AStaticMeshActor* Pillar = SpawnDecoActor(World, CylinderMesh, Loc, FRotator::ZeroRotator,
			FVector(1.4f, 1.4f, 2.4f), StoneColor, 0.15f))
		{
			// Cap stone
			SpawnDecoActor(World, CubeMesh, Loc + FVector(0.f, 0.f, 260.f), FRotator::ZeroRotator,
				FVector(2.2f, 2.2f, 0.35f), StoneColor * 1.1f, 0.1f);
		}

		// Every third pillar gets a torch
		if (i % 3 == 0)
		{
			const FVector TorchLoc = Loc + FRotator(0.f, Angle, 0.f).Vector() * -180.f + FVector(0.f, 0.f, 280.f);
			if (AStaticMeshActor* Torch = SpawnDecoActor(World, CylinderMesh, TorchLoc, FRotator::ZeroRotator,
				FVector(0.35f, 0.35f, 0.9f), AccentGold, 2.f))
			{
				AttachTorchLight(Torch, FLinearColor(1.f, 0.55f, 0.15f), 12000.f);
			}
			if (ConeMesh)
			{
				SpawnDecoActor(World, ConeMesh, TorchLoc + FVector(0.f, 0.f, 100.f), FRotator::ZeroRotator,
					FVector(0.5f, 0.5f, 0.7f), AccentGold, 3.f);
			}
		}
	}

	// --- Four cardinal gate arches ---
	const float GateRadius = ArenaHalfSize * 0.55f;
	const FLinearColor GateColors[] = { AccentBlue, AccentGold, AccentRed, AccentBlue };
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = 90.f * i;
		const FVector GateCenter = FRotator(0.f, Angle, 0.f).Vector() * GateRadius + FVector(0.f, 0.f, 160.f);
		const FRotator GateRot(0.f, Angle, 0.f);

		SpawnDecoActor(World, CubeMesh, GateCenter + GateRot.RotateVector(FVector(-120.f, 0.f, 0.f)),
			GateRot, FVector(0.6f, 0.6f, 3.2f), GateColors[i], 0.6f);
		SpawnDecoActor(World, CubeMesh, GateCenter + GateRot.RotateVector(FVector(120.f, 0.f, 0.f)),
			GateRot, FVector(0.6f, 0.6f, 3.2f), GateColors[i], 0.6f);
		SpawnDecoActor(World, CubeMesh, GateCenter + FVector(0.f, 0.f, 320.f),
			GateRot, FVector(3.2f, 0.5f, 0.5f), GateColors[i], 0.8f);
	}

	// --- Corner accent braziers with warm light ---
	for (int32 i = 0; i < 4; ++i)
	{
		const float Angle = 45.f + 90.f * i;
		const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * (ArenaHalfSize * 0.65f) + FVector(0.f, 0.f, 30.f);
		if (AStaticMeshActor* Brazier = SpawnDecoActor(World, CylinderMesh, Loc, FRotator::ZeroRotator,
			FVector(1.8f, 1.8f, 0.5f), AccentRed * 0.7f, 0.5f))
		{
			AttachTorchLight(Brazier, FLinearColor(1.f, 0.35f, 0.1f), 18000.f);
		}
	}
}
