extends RefCounted
## Pixel units and seconds. Sources: BuiltInData.cpp, RecoilMovementTuning.h,
## TileCollisionResolver.h and MobileTuning.h in the raylib game.

const HALF := 16.0
const TILE := 32.0
const GRAVITY := 1800.0
const APEX := 120.0
const APEX_GRAVITY := 0.55
const RISE_DRAG := 0.22
const AIR_DRAG := 0.25
const GROUND_DRAG := 0.035
const BRACE_DRAG := 0.0015
const BRACE_X := 0.20
const BRACE_UP := 0.12
const BRACE_ANCHOR := 90.0
const SYNERGY := 0.2
const SNAP := 6.0
const MAX_X := 3000.0
const MAX_UP := 2000.0
const MAX_FALL := 2000.0
const MAX_STOMP := 3000.0

const PAD_RADIUS := 100.0
const KNOB_RADIUS := 28.0
const DEAD_ZONE := 18.0
const PAD_MARGIN := Vector2(50, 56)
const SLOW := 0.08
const DEFAULT_ZOOM := 1.10
const MIN_ZOOM := 0.65
const MAX_ZOOM := 1.80

const GUNS := [
	{"name": "REVOLVER", "ammo": 6, "recoil": 600.0, "reload": 0.4,
	"cooldown": 0.3, "speed": 900.0, "range": 900.0, "count": 1, "spread": 0.0,
	"damage": 12, "stop": 0.065, "shake": 11.0, "shake_time": 0.08, "resist": 0.05},
	{"name": "SHOTGUN", "ammo": 2, "recoil": 950.0, "reload": 0.75,
	"cooldown": 0.35, "speed": 820.0, "range": 285.0, "count": 7, "spread": 28.0,
	"damage": 6, "stop": 0.12, "shake": 18.0, "shake_time": 0.24, "resist": 0.45},
]
