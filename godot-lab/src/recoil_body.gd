extends RefCounted
## All movement and hit boxes remain continuous. Only this sprite clock is stepped.
const T = preload("res://src/tuning.gd")
const Data = preload("res://src/recoil_cel_data.gd")
const ART = preload("res://assets/recoil-cels.svg")
const IDLE := 24
const PIXEL := 2.0
const CELL := Vector2(Data.CELL, Data.CELL)
const HALF_CELL := CELL * 0.5
const SETTLE := 0.065
const LAND_STEP := 0.055

enum { REST, RECOIL, LAND }
var state := REST
var octant := 0
var age := 0.0
var squeeze_time := 0.0
var stretch_time := 0.0
var hard_landing := false

func launch(impulse: Vector2, hit_stop: float, heavy := false) -> void:
	if impulse.length_squared() < 1.0:
		return
	octant = posmod(roundi(impulse.angle() / (PI * 0.25)), Data.COLS)
	squeeze_time = maxf(hit_stop, 0.05)
	stretch_time = 0.13 if heavy else 0.085
	age = 0.0
	state = RECOIL

func land(speed: float) -> void:
	if speed < 100.0:
		return
	hard_landing = speed >= 640.0
	age = 0.0
	state = LAND

func advance(dt: float) -> void:
	age += dt

func frame_id() -> int:
	match state:
		RECOIL:
			if age < squeeze_time:
				return octant
			if age < squeeze_time + stretch_time:
				return Data.COLS + octant
			if age < squeeze_time + stretch_time + SETTLE:
				return 2 * Data.COLS + octant
		LAND:
			var step := floori(age / LAND_STEP) + (0 if hard_landing else 1)
			if step < 3:
				return IDLE + 1 + step
	return IDLE

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
