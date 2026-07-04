# MultiGame - LAN Co-op Third Person Action Game (UE 5.5, C++)

Local (LAN) co-op aksiyon oyunu. Bir oyuncu **Host** olur (listen server), diğerleri **IP ile** bağlanır. İçerik:

- LAN multiplayer (server-authoritative)
- Karakter yaratma + görsel customization (mesh + renk), diske kayıt
- Saldırı hamleleri (light combo + heavy), animasyon tabanlı hitbox
- Hareket hamleleri (sprint, dodge/roll + i-frame)
- İki düşman tipi: **melee** ve **caster** (projectile)
- Fazlı **boss fight** (HP eşiklerine göre faz değişimi + wave saldırıları)

## Gereksinimler

- Unreal Engine **5.5**
- Visual Studio 2022 (C++ game dev workload)

## Derleme

1. `MultiGame.uproject` dosyasına sağ tıkla -> **Generate Visual Studio project files**.
2. `MultiGame.sln` dosyasını aç, **Development Editor / Win64** ile derle.
3. `MultiGame.uproject` ile editörü aç.

## C++ Mimari Özeti

- `Core/` - `MultiGameInstance` (LAN host/join + appearance kaydı), `MultiGameMode` (wave + boss + win/lose), `MultiGameState` (maç fazı/wave/boss), `MultiGamePlayerState` (replike appearance), `MultiGamePlayerController` (appearance/ready RPC + UI hook)
- `Characters/` - `MultiGameCharacter` (base: health, replike montage, ölüm), `PlayerCharacter` (Enhanced Input, combat/ability/movement, customization)
- `Components/` - `HealthComponent`, `CombatComponent` (melee combo + trace), `AbilityComponent` (cooldown + projectile)
- `Combat/` - `ProjectileBase`, `MeleeHitboxNotifyState` (hasar penceresi notify)
- `Enemies/` - `EnemyBase`, `EnemyAIController` (perception + C++ state machine), `MeleeEnemy`, `CasterEnemy`, `BossCharacter`
- `UI/` - `MainMenuWidget`, `LobbyWidget`, `CharacterCreationWidget`, `HUDWidget` (UUserWidget base'leri)
- `Save/` - `PlayerCharacterSaveGame`

## Editörde Yapılması Gereken Asset'ler (binary, kodla oluşturulamaz)

Kod hazır; oynanabilir hale getirmek için editörde şu asset'leri oluşturup C++ sınıflarından türet:

### 1. Haritalar (`Content/MultiGame/Maps/`)
- `MainMenu` - ana menü haritası (varsayılan açılış haritası)
- `Lobby` - lobi haritası
- `Arena` - oyun/boss arenası. Zemine `NavMeshBoundsVolume` ekle (AI hareketi için). Düşmanların doğacağı yerlere boş Actor koyup **Tag = `EnemySpawn`** ver. `PlayerStart` ekle.

`Config/DefaultEngine.ini` bu isimleri `/Game/MultiGame/Maps/` altında bekler.

### 2. Enhanced Input (`Content/MultiGame/Input/`)
- `IMC_Player` (Input Mapping Context)
- Input Action'lar: `IA_Move` (Axis2D), `IA_Look` (Axis2D), `IA_Jump`, `IA_Sprint`, `IA_Dodge`, `IA_LightAttack`, `IA_HeavyAttack`, `IA_Ability1`, `IA_Ability2`
- Bunları `BP_PlayerCharacter` içindeki ilgili alanlara ata.

### 3. Blueprint'ler
- `BP_PlayerCharacter` : `PlayerCharacter`
  - `GetMesh`'e iskelet mesh + `AnimBP` ata; `BodyMeshOptions` / `HeadMeshOptions` listelerini doldur.
  - Material'de `PrimaryColor` / `SecondaryColor` vektör parametreleri olsun (customization renk için).
  - Combat/Ability montage ve projectile alanlarını doldur.
- `BP_MeleeEnemy` : `MeleeEnemy`, `BP_CasterEnemy` : `CasterEnemy`, `BP_Boss` : `BossCharacter`
- `BP_Fireball` : `ProjectileBase`
- Attack montage'larına **Melee Hitbox Window** (`MeleeHitboxNotifyState`) notify state'ini aktif kare aralığına ekle.
- `GameMode` (BP veya defaults) içinde `MeleeEnemyClass`, `CasterEnemyClass`, `BossClass` ata.

### 4. UMG Widget'ları
- `WBP_MainMenu` : `MainMenuWidget` (Host/Join/Quit butonları -> `OnHostClicked`/`OnJoinClicked`/`OnQuitClicked`)
- `WBP_Lobby` : `LobbyWidget`
- `WBP_CharacterCreation` : `CharacterCreationWidget`
- `WBP_HUD` : `HUDWidget` (progress bar'ları `GetLocalHealthPercent`/`GetBossHealthPercent`/`GetCurrentWave`'e bağla)
- `MainMenu` haritasında bir controller/level BP ile `WBP_MainMenu`'yu ekranda göster. `MultiGamePlayerController::ShowGameplayHUD` event'ini `WBP_HUD` oluşturacak şekilde implement et.

## Test (LAN)

- Editörde **Play** -> Net Mode: **Play As Listen Server**, oyuncu sayısı 2+.
- Gerçek LAN'de: Host makinede oyunu aç -> Host. Diğer makinede -> Join, host IP'sini gir.
