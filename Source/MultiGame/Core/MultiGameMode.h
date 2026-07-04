#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "MultiGameMode.generated.h"

class AEnemyBase;
class ABossCharacter;
class AMultiGameCharacter;

/**
 * Server-authoritative encounter driver: spawns enemy waves, then a boss, and
 * resolves win/lose. Also applies replicated appearance to spawned player pawns.
 */
UCLASS()
class MULTIGAME_API AMultiGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AMultiGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;

protected:
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void BeginPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	void StartMatch();
	void SpawnWave(int32 WaveIndex);
	void SpawnBoss();

	/** Builds floor, lights, player/enemy spawn points (safe before player login). */
	void EnsureSpawnInfrastructure();

	/** Builds nav mesh after the world has fully initialized (BeginPlay + delay). */
	void EnsureNavigationMesh();

	FTransform GetEnemySpawnTransform() const;

	UFUNCTION()
	void OnEnemyDied(AMultiGameCharacter* Enemy);

	UFUNCTION()
	void OnPlayerDied(AMultiGameCharacter* Player);

	void CheckWaveCleared();
	void EndMatch(bool bPlayersWon);

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Enemies")
	TSubclassOf<AEnemyBase> MeleeEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Enemies")
	TSubclassOf<AEnemyBase> CasterEnemyClass;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Enemies")
	TSubclassOf<ABossCharacter> BossClass;

	/** Number of standard waves before the boss appears. */
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	int32 TotalWaves = 3;

	/** Melee enemies spawned in wave N = BaseMelee + WaveIndex. */
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	int32 BaseMeleePerWave = 1;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	int32 BaseCasterPerWave = 0;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	float DelayBeforeStart = 3.f;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	float DelayBetweenWaves = 6.f;

	/** Tag placed on actors used as enemy spawn points in the level. */
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Waves")
	FName EnemySpawnTag = TEXT("EnemySpawn");

	/** If true, generate a floor/nav/lights/spawns when none exist in the level. */
	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Arena")
	bool bAutoBuildArena = true;

	UPROPERTY(EditDefaultsOnly, Category = "MultiGame|Arena")
	float ArenaHalfSize = 3500.f;

	bool bSpawnInfrastructureBuilt = false;
	bool bNavigationBuilt = false;

	int32 CurrentWaveIndex = 0;
	int32 EnemiesAlive = 0;
	int32 PlayersAlive = 0;
	bool bMatchStarted = false;

	FTimerHandle StartTimer;
	FTimerHandle NextWaveTimer;
	FTimerHandle NavBuildTimer;
};
