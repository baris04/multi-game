# MultiGame — Game Design Document

**Genre:** LAN co-op third-person action  
**Engine:** Unreal Engine 5.5 (C++)  
**Players:** 2+ (listen-server host + IP join)  
**Camera:** Third-person, over-the-shoulder

---

## Space / Story

**Setting:** A combat arena — open stone platform, columns, torches, fog, and directional lighting. If the level has no geometry, the game builds floor, spawn points, navigation, and lighting at runtime.

**Premise:** Heroes drop into an enemy-held arena over LAN. There is no scripted narrative; the fantasy is simple — survive the waves, then kill the boss. Character creation (name, class, mesh, colors) is cosmetic and saved locally; it does not change story branches.

**Session flow:** Main Menu → (optional) Character Creation → Lobby → Arena encounter.

---

## Actors

| Actor | Role |
|--------|------|
| **Player (`APlayerCharacter`)** | Third-person hero. 500 HP, reduced incoming damage. Melee combo + heavy attack, two projectile abilities, sprint, dodge with i-frames. Custom appearance (Gideon / Manny mesh, tint). |
| **Melee Enemy (`AMeleeEnemy`)** | Close-range grunt. 160 HP, red-tinted Gideon look. Light melee (4 dmg) / heavy (7 dmg). Chases to ~120 uu. |
| **Caster Enemy (`ACasterEnemy`)** | Ranged grunt. 110 HP, skeleton look. Fires projectiles (6 dmg, 4s cooldown). Keeps ~700 uu distance, retreats if pressed. |
| **Boss (`ABossCharacter`)** | 350 HP finale. Multi-phase at 66% / 33% HP: faster attacks, melee toggle, projectile rings (4 then 6 bolts). |
| **Projectile (`AProjectileBase`)** | Shared by players, casters, and boss waves. Server-spawned, replicated visually. |
| **Game Mode (`AMultiGameMode`)** | Server authority: waves, spawns, win/lose, player appearance on login. |
| **Game State (`AMultiGameState`)** | Replicated match phase, wave index, boss reference for HUD. |
| **AI Controller (`AEnemyAIController`)** | Perception + state machine: find nearest hostile player, move, attack in range. |

**Teams:** `Players` vs `Enemies` — only cross-team damage counts.

---

## Goals

**Primary (win):** Clear **3 enemy waves**, then **defeat the boss**.

**Secondary:** Survive as a group; coordinate melee pressure and dodge/projectile avoidance.

**Failure (lose):** **All players die** — the match ends immediately.

**Player-facing objectives (HUD):** Current wave, boss health during boss fight, personal health.

---

## Mechanics

### Multiplayer
- **Listen server** host; clients join by **IP**.
- Combat, damage, spawning, and match state are **server-authoritative**; animations and visuals replicate to clients.

### Player combat & movement
- **Light attack:** 3-step combo (35 dmg per hit), combo window ~1.2s.
- **Heavy attack:** 65 dmg single hit.
- **Abilities (×2):** Cooldown-gated projectile casts toward camera aim.
- **Sprint:** Walk 500 → sprint 850 uu/s.
- **Dodge:** Impulse roll, ~0.5s invulnerability.
- **Locomotion:** Idle ↔ jog/walk based on speed; attack animations pause locomotion.

### Enemy AI
- Scans for nearest living hostile player within sight radius.
- **Melee:** closes to attack range (~200 uu), light attack on cooldown (~3.5s).
- **Caster:** stays at range (~1000 uu attack), backs away if too close.
- Floating **health bars** on enemies.

### Encounter loop
1. **3s** delay after match start.
2. **Wave N:** `1 + N` melee + `N/2` casters (defaults: 1→2→3 melee, 0→0→1 caster).
3. **6s** between waves after clear.
4. After wave 3 → **Boss Fight** phase.
5. Boss death → **Won**; all players dead → **Lost**.

### Boss phases
- **Phase 1:** Baseline (4s attack cooldown, no projectile ring).
- **Phase 2 (≤66% HP):** 3s cooldown, 4-projectile ring.
- **Phase 3 (≤33% HP):** 2.5s cooldown, 6-projectile ring, alternates melee patterns.

### Customization
- Name, class (Warrior / Mage / Rogue — cosmetic), body mesh index, primary/secondary colors.
- Saved to disk via `PlayerCharacterSaveGame`.

---

## Rules

Player-facing rules — how to play.

### Objective
- **Win:** Clear all enemies in **3 waves**, then **defeat the boss**.
- **Lose:** If **every player on the team dies**, the match ends. There is no respawn.

### Players & connection
- **LAN co-op:** one player **Hosts**; others **Join** using the host’s IP address.
- Everyone fights in the same arena. The **Host runs the match** (server-authoritative).

### Match flow
1. Match starts → short prep period.
2. **Waves 1, 2, and 3** play in order. A wave ends only when **all enemies in that wave are dead**.
3. After Wave 3 → **Boss fight** begins.
4. Kill the boss → **Victory**. Entire team wiped → **Defeat**.

### Movement & combat
- **Move** with WASD; **look** with the camera.
- **Sprint** for faster movement.
- **Dodge (roll)** grants brief **invulnerability** — use it to avoid damage.
- **Light attack:** combo chain (chain hits within a short timing window).
- **Heavy attack:** slower, stronger single hit.
- **Abilities (×2):** fire projectiles; each has a **cooldown** — cannot recast until it expires.

### Enemy types

| Enemy | Behavior |
|--------|----------|
| **Melee** | Closes in and strikes at close range. |
| **Caster** | Fights from distance with projectiles; backs away if you get too close. |
| **Boss** | Gets **more aggressive** as health drops; phase changes increase attack pace and projectile waves. |

Enemies display a **health bar** above them.

### Damage & survival
- **No friendly fire** — you only damage enemies.
- When you die, you are **out for the rest of the match** (no respawn).
- **Dodge** grants temporary invulnerability.
- Players are tuned to survive longer than a single hit, but sustained pressure from multiple enemies or the boss can still kill you.

### Character creation
- Choose **name**, **class** (Warrior / Mage / Rogue), **appearance**, and **colors**.
- Class choice is **cosmetic** — core gameplay is the same for everyone (melee + 2 abilities).
- Appearance is **saved to disk** and can be reused in later sessions.

### Arena
- Combat takes place in a **single open arena**.
- Enemies spawn in **waves** from arena edges / spawn points.
- Positioning matters: engage melee enemies up close, dodge caster and boss projectiles.

### One-line summary
**Team up over LAN, survive three waves, kill the boss — if everyone dies, you lose.**
