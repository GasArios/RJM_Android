extends RefCounted
## All movement and hit boxes remain continuous. Only this sprite clock is stepped.
const T = preload("res://src/tuning.gd")
const Data = preload("res://src/recoil_cel_data.gd")
const ART = preload("res://assets/recoil-cels.svg")
const IDLE := Data.IDLE
const PIXEL := 2.0
const CELL := Vector2(Data.CELL, Data.CELL)
const HALF_CELL := CELL * 0.5
const MIN_KICK := 80.0
const WEAK_KICK := 320.0
const HEAVY_KICK := 800.0
const HEAVY_PRESS := 0.050
const REBOUND_STEP := 0.042
const LIGHT_RECOVERY := 0.056
const HEAVY_RECOVERY := 0.090
const LAND_STEP := 0.035

enum { REST, RECOIL, LAND }
enum { PRESS, STRONG_PRESS, STRONG_REBOUND, SOFT_REBOUND }
var state := REST
var octant := 0
var age := 0.0
var release_at := 0.0
var kick_strength := 0.0
var landing_start := 0

func launch(impulse: Vector2, hit_stop: float) -> void:
	var strength := impulse.length()
	if strength < MIN_KICK:
		return
	kick_strength = strength
	octant = posmod(roundi(impulse.angle() / (PI * 0.25)), Data.COLS)
	release_at = hit_stop
	age = 0.0
	state = RECOIL

func land(speed: float) -> void:
	if speed < 160.0:
		return
	landing_start = 0 if speed >= 700.0 else (1 if speed >= 400.0 else 2)
	age = 0.0
	state = LAND

func advance(dt: float) -> void:
	age += dt

func frame_id() -> int:
	match state:
		RECOIL:
			if kick_strength < WEAK_KICK:
				return _directional(PRESS) if age < HEAVY_PRESS else IDLE
			var heavy := kick_strength >= HEAVY_KICK
			if age < release_at:
				return _directional(STRONG_PRESS if heavy and age < HEAVY_PRESS else PRESS)
			var after_release := age - release_at
			if heavy and after_release < REBOUND_STEP:
				return _directional(STRONG_REBOUND)
			if after_release < (HEAVY_RECOVERY if heavy else LIGHT_RECOVERY):
				return _directional(SOFT_REBOUND)
		LAND:
			var step := landing_start + floori(age / LAND_STEP)
			if step < 3:
				return IDLE + 1 + step
	return IDLE

func _directional(phase: int) -> int:
	return phase * Data.COLS + octant

func sprite_center(center: Vector2, grounded: bool, frame: int) -> Vector2:
	if grounded:
		center.y += T.HALF - float(Data.BOTTOM[frame]) * PIXEL
	return center

func draw_at(canvas: CanvasItem, center: Vector2, grounded: bool, frame: int) -> void:
	var base := sprite_center(center, grounded, frame)
	var cell := Vector2(frame % Data.COLS, floori(float(frame) / Data.COLS))
	canvas.draw_texture_rect_region(ART, Rect2(base - HALF_CELL * PIXEL, CELL * PIXEL),
			Rect2(cell * CELL, CELL))

func edge(center: Vector2, grounded: bool, frame: int, direction: Vector2) -> Vector2:
	# Follow the actual silhouette with a short pixel ray. This is only the
	# barrel drawing anchor; projectile origin and aim still use the physics box.
	var base := sprite_center(center, grounded, frame)
	var rows: String = Data.SPANS[frame]
	for sample in range(Data.CELL * 3):
		var dist := float(sample) * 0.5
		var pixel := Vector2i((HALF_CELL + direction * dist).floor())
		if (pixel.y < 0 or pixel.y >= Data.CELL
				or pixel.x < rows.unicode_at(pixel.y * 2) - 65
				or pixel.x >= rows.unicode_at(pixel.y * 2 + 1) - 65):
			return base + direction * (maxf(0.0, dist - 0.25) * PIXEL)
	return base
