#include "Core/MultiGameMode.h"
#include "Core/MultiGameSceneSetup.h"
#include "Core/MultiGameState.h"
#include "Core/MultiGamePlayerState.h"
#include "Core/MultiGamePlayerController.h"
#include "Characters/PlayerCharacter.h"
#include "Enemies/EnemyBase.h"
#include "Enemies/MeleeEnemy.h"
#include "Enemies/CasterEnemy.h"
#include "Enemies/BossCharacter.h"
#include "Engine/World.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/SkyLight.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "GameFramework/PlayerStart.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AMultiGameMode::AMultiGameMode()
{
	GameStateClass = AMultiGameState::StaticClass();
	PlayerStateClass = AMultiGamePlayerState::StaticClass();
	PlayerControllerClass = AMultiGamePlayerController::StaticClass();
	DefaultPawnClass = APlayerCharacter::StaticClass();
	bStartPlayersAsSpectators = false;

	// Default to the C++ enemy classes so the game works without Blueprints.
	MeleeEnemyClass = AMeleeEnemy::StaticClass();
	CasterEnemyClass = ACasterEnemy::StaticClass();
	BossClass = ABossCharacter::StaticClass();
}

void AMultiGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);

	// PlayerStarts must exist before the first RestartPlayer(); nav mesh is deferred to BeginPlay.
	EnsureSpawnInfrastructure();
}

void AMultiGameMode::BeginPlay()
{
	Super::BeginPlay();

	MultiGameSceneSetup::ApplyGameplayLighting(GetWorld());

	// Re-capture skylight after atmosphere spawns.
	if (UWorld* World = GetWorld())
	{
		TArray<AActor*> SkyLights;
		UGameplayStatics::GetAllActorsOfClass(World, ASkyLight::StaticClass(), SkyLights);
		for (AActor* Actor : SkyLights)
		{
			if (ASkyLight* Sky = Cast<ASkyLight>(Actor))
			{
				if (USkyLightComponent* Light = Sky->GetLightComponent())
				{
					Light->RecaptureSky();
				}
			}
		}
	}

	// Fallback: start waves even if PostLogin already ran (common in PIE).
	if (!bMatchStarted && !GetWorldTimerManager().IsTimerActive(StartTimer))
	{
		GetWorldTimerManager().SetTimer(StartTimer, this, &AMultiGameMode::StartMatch, DelayBeforeStart, false);
	}

	if (bAutoBuildArena && !bNavigationBuilt)
	{
		GetWorldTimerManager().SetTimer(NavBuildTimer, this, &AMultiGameMode::EnsureNavigationMesh, 1.f, false);
	}
}

void AMultiGameMode::RestartPlayer(AController* NewPlayer)
{
	EnsureSpawnInfrastructure();

	Super::RestartPlayer(NewPlayer);

	if (APlayerController* PC = Cast<APlayerController>(NewPlayer))
	{
		if (APlayerCharacter* PlayerChar = Cast<APlayerCharacter>(PC->GetPawn()))
		{
			++PlayersAlive;
			PlayerChar->OnCharacterDied.AddDynamic(this, &AMultiGameMode::OnPlayerDied);
		}
	}
}

void AMultiGameMode::EnsureSpawnInfrastructure()
{
	UWorld* World = GetWorld();
	if (!World || bSpawnInfrastructureBuilt)
	{
		return;
	}
	bSpawnInfrastructureBuilt = true;

	TArray<AActor*> ExistingMeshes;
	UGameplayStatics::GetAllActorsOfClass(World, AStaticMeshActor::StaticClass(), ExistingMeshes);

	UStaticMesh* CubeMesh = LoadObject<UStaticMesh>(nullptr, TEXT("/Engine/BasicShapes/Cube.Cube"));

	if (ExistingMeshes.Num() == 0 && CubeMesh)
	{
		FActorSpawnParameters Params;
		AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(),
			FVector(0.f, 0.f, -10.f), FRotator::ZeroRotator, Params);
		if (Floor)
		{
			Floor->SetMobility(EComponentMobility::Static);
			if (UStaticMeshComponent* SMC = Floor->GetStaticMeshComponent())
			{
				SMC->SetStaticMesh(CubeMesh);
				const float Scale = (ArenaHalfSize * 2.f) / 100.f;
				SMC->SetWorldScale3D(FVector(Scale, Scale, 0.2f));
				SMC->SetMobility(EComponentMobility::Static);

				if (UMaterial* FloorMat = LoadObject<UMaterial>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial")))
				{
					SMC->SetMaterial(0, FloorMat);
					if (UMaterialInstanceDynamic* MID = SMC->CreateAndSetMaterialInstanceDynamic(0))
					{
						const FLinearColor FloorColor(0.22f, 0.24f, 0.28f);
						MID->SetVectorParameterValue(TEXT("Color"), FloorColor);
						MID->SetVectorParameterValue(TEXT("BaseColor"), FloorColor);
						MID->SetVectorParameterValue(TEXT("EmissiveColor"), FLinearColor(0.08f, 0.10f, 0.14f));
					}
				}
			}
		}
	}

	MultiGameSceneSetup::ApplyGameplayLighting(World);
	MultiGameSceneSetup::ApplyArenaDecoration(World, ArenaHalfSize);

	TArray<AActor*> Starts;
	UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), Starts);
	if (Starts.Num() == 0)
	{
		const int32 NumStarts = 4;
		const float SpawnZ = MultiGameSceneSetup::GetCharacterSpawnZ();
		// Spawn outside the center platform disc (radius ~250) on the open floor.
		const float PlayerSpawnRadius = 450.f;
		for (int32 i = 0; i < NumStarts; ++i)
		{
			const float Angle = (360.f / NumStarts) * i;
			const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * PlayerSpawnRadius + FVector(0.f, 0.f, SpawnZ);
			World->SpawnActor<APlayerStart>(APlayerStart::StaticClass(), Loc, FRotator(0.f, Angle + 180.f, 0.f));
		}
	}

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(World, EnemySpawnTag, SpawnPoints);
	if (SpawnPoints.Num() == 0)
	{
		const int32 NumPoints = 6;
		const float SpawnZ = MultiGameSceneSetup::GetCharacterSpawnZ();
		for (int32 i = 0; i < NumPoints; ++i)
		{
			// Offset the enemy ring by half a step so enemies never align with a player start,
			// and push them right out to the arena edge so players don't spawn near them.
			const float Angle = (360.f / NumPoints) * i + 30.f;
			const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * (ArenaHalfSize * 0.95f) + FVector(0.f, 0.f, SpawnZ);
			if (AActor* Point = World->SpawnActor<AActor>(AActor::StaticClass(), Loc, FRotator::ZeroRotator))
			{
				USceneComponent* Root = NewObject<USceneComponent>(Point, TEXT("Root"));
				Root->RegisterComponent();
				Point->SetRootComponent(Root);
				Point->Tags.Add(EnemySpawnTag);
			}
		}
	}
}

void AMultiGameMode::EnsureNavigationMesh()
{
	if (bNavigationBuilt || !bAutoBuildArena)
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	if (!NavSys)
	{
		return;
	}

	bNavigationBuilt = true;

	if (ANavMeshBoundsVolume* NavVolume = World->SpawnActor<ANavMeshBoundsVolume>(
		ANavMeshBoundsVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator))
	{
		NavVolume->SetActorScale3D(FVector(ArenaHalfSize / 100.f, ArenaHalfSize / 100.f, 4.f));
		NavVolume->GetRootComponent()->UpdateBounds();
		NavSys->OnNavigationBoundsUpdated(NavVolume);
	}
}

void AMultiGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	// Kick off the encounter once the first player has joined.
	if (!bMatchStarted && !GetWorldTimerManager().IsTimerActive(StartTimer))
	{
		GetWorldTimerManager().SetTimer(StartTimer, this, &AMultiGameMode::StartMatch, DelayBeforeStart, false);
	}
}

void AMultiGameMode::StartMatch()
{
	if (bMatchStarted)
	{
		return;
	}
	bMatchStarted = true;

	if (AMultiGameState* GS = GetGameState<AMultiGameState>())
	{
		GS->SetMatchPhase(EMatchPhase::InProgress);
	}

	CurrentWaveIndex = 0;
	SpawnWave(1);
}

FTransform AMultiGameMode::GetEnemySpawnTransform() const
{
	UWorld* World = GetWorld();

	// Gather current player pawn locations so we can bias spawns away from them.
	TArray<FVector> PlayerLocations;
	if (World)
	{
		for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (APawn* Pawn = PC->GetPawn())
				{
					PlayerLocations.Add(Pawn->GetActorLocation());
				}
			}
		}
	}

	auto DistanceToNearestPlayer = [&PlayerLocations](const FVector& Location) -> float
	{
		if (PlayerLocations.Num() == 0)
		{
			return TNumericLimits<float>::Max();
		}
		float Nearest = TNumericLimits<float>::Max();
		for (const FVector& PlayerLoc : PlayerLocations)
		{
			Nearest = FMath::Min(Nearest, FVector::Dist(PlayerLoc, Location));
		}
		return Nearest;
	};

	TArray<AActor*> SpawnPoints;
	UGameplayStatics::GetAllActorsWithTag(World, EnemySpawnTag, SpawnPoints);

	if (SpawnPoints.Num() > 0)
	{
		// Prefer spawn points that are far from every player; pick randomly among the
		// farthest few so waves still vary but never land on top of the players.
		SpawnPoints.Sort([&DistanceToNearestPlayer](const AActor& A, const AActor& B)
		{
			return DistanceToNearestPlayer(A.GetActorLocation()) > DistanceToNearestPlayer(B.GetActorLocation());
		});

		const int32 CandidateCount = FMath::Clamp(SpawnPoints.Num() / 2, 1, SpawnPoints.Num());
		AActor* Point = SpawnPoints[FMath::RandRange(0, CandidateCount - 1)];
		return Point->GetActorTransform();
	}

	// Fallback: ring far out around world origin.
	const float Angle = FMath::FRandRange(0.f, 360.f);
	const FVector Loc = FRotator(0.f, Angle, 0.f).Vector() * (ArenaHalfSize * 0.95f)
		+ FVector(0.f, 0.f, MultiGameSceneSetup::GetCharacterSpawnZ());
	return FTransform(FRotator::ZeroRotator, Loc);
}

void AMultiGameMode::SpawnWave(int32 WaveIndex)
{
	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	CurrentWaveIndex = WaveIndex;
	EnemiesAlive = 0;

	if (AMultiGameState* GS = GetGameState<AMultiGameState>())
	{
		GS->SetCurrentWave(WaveIndex);
	}

	const int32 NumMelee = BaseMeleePerWave + WaveIndex;
	const int32 NumCaster = BaseCasterPerWave + (WaveIndex / 2);

	auto SpawnEnemy = [&](TSubclassOf<AEnemyBase> Class)
	{
		if (!Class)
		{
			return;
		}

		const FTransform Xform = GetEnemySpawnTransform();
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (AEnemyBase* Enemy = World->SpawnActor<AEnemyBase>(Class, Xform, Params))
		{
			++EnemiesAlive;
			Enemy->OnCharacterDied.AddDynamic(this, &AMultiGameMode::OnEnemyDied);

			if (HasAuthority() && !Enemy->GetController())
			{
				Enemy->SpawnDefaultController();
			}
		}
	};

	for (int32 i = 0; i < NumMelee; ++i)  { SpawnEnemy(MeleeEnemyClass); }
	for (int32 i = 0; i < NumCaster; ++i) { SpawnEnemy(CasterEnemyClass); }

	// If nothing spawned (missing classes), advance so we don't soft-lock.
	if (EnemiesAlive == 0)
	{
		CheckWaveCleared();
	}
}

void AMultiGameMode::SpawnBoss()
{
	UWorld* World = GetWorld();
	if (!World || !BossClass)
	{
		// No boss configured: players win after final wave.
		EndMatch(true);
		return;
	}

	if (AMultiGameState* GS = GetGameState<AMultiGameState>())
	{
		GS->SetMatchPhase(EMatchPhase::BossFight);
	}

	const FTransform Xform = GetEnemySpawnTransform();
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ABossCharacter* Boss = World->SpawnActor<ABossCharacter>(BossClass, Xform, Params))
	{
		EnemiesAlive = 1;
		Boss->OnCharacterDied.AddDynamic(this, &AMultiGameMode::OnEnemyDied);

		if (HasAuthority() && !Boss->GetController())
		{
			Boss->SpawnDefaultController();
		}

		if (AMultiGameState* GS = GetGameState<AMultiGameState>())
		{
			GS->SetBoss(Boss);
		}
	}
}

void AMultiGameMode::OnEnemyDied(AMultiGameCharacter* Enemy)
{
	EnemiesAlive = FMath::Max(0, EnemiesAlive - 1);
	CheckWaveCleared();
}

void AMultiGameMode::CheckWaveCleared()
{
	if (EnemiesAlive > 0)
	{
		return;
	}

	AMultiGameState* GS = GetGameState<AMultiGameState>();
	if (GS && GS->GetMatchPhase() == EMatchPhase::BossFight)
	{
		EndMatch(true);
		return;
	}

	if (CurrentWaveIndex >= TotalWaves)
	{
		// All waves cleared: bring in the boss.
		GetWorldTimerManager().SetTimer(NextWaveTimer, this, &AMultiGameMode::SpawnBoss, DelayBetweenWaves, false);
	}
	else
	{
		const int32 Next = CurrentWaveIndex + 1;
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &AMultiGameMode::SpawnWave, Next);
		GetWorldTimerManager().SetTimer(NextWaveTimer, Delegate, DelayBetweenWaves, false);
	}
}

void AMultiGameMode::OnPlayerDied(AMultiGameCharacter* Player)
{
	PlayersAlive = FMath::Max(0, PlayersAlive - 1);
	if (PlayersAlive <= 0)
	{
		EndMatch(false);
	}
}

void AMultiGameMode::EndMatch(bool bPlayersWon)
{
	if (AMultiGameState* GS = GetGameState<AMultiGameState>())
	{
		if (GS->GetMatchPhase() == EMatchPhase::Won || GS->GetMatchPhase() == EMatchPhase::Lost)
		{
			return;
		}
		GS->SetMatchPhase(bPlayersWon ? EMatchPhase::Won : EMatchPhase::Lost);
	}

	GetWorldTimerManager().ClearTimer(NextWaveTimer);
}
