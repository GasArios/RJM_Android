extends RefCounted
const T = preload("res://src/tuning.gd")
## Progression and fixed checkpoints. World drawing and platform input live elsewhere.
enum { FIRST_FLIGHT, LOW_PIER, SUPPLY, UPPER_PIER, BEACON, COMPLETE }

const START := Vector2(220, 816)
const PIER_CHECKPOINT := Vector2(408, 784)
const SUPPLY_POINT := Vector2(608, 784)
const UPPER_CHECKPOINT := Vector2(1016, 656)
const BEACON_POINT := Vector2(2208, 528)
const FIRST_WALL := 320.0
const UPPER_WALL := 896.0
const ROOF_WALL := 1472.0
const BEACON_WALL := 1856.0
const FORMAT := 1

var stage := FIRST_FLIGHT
var file_path := "user://harbor-route.cfg"

func reset() -> void:
	stage = FIRST_FLIGHT

func load_save() -> void:
	reset()
	var file := ConfigFile.new()
	if file.load(file_path) != OK or file.get_value("harbor", "format", 0) != FORMAT:
		return
	var stored: Variant = file.get_value("harbor", "stage", FIRST_FLIGHT)
	if typeof(stored) == TYPE_INT and stored >= FIRST_FLIGHT and stored <= COMPLETE:
		stage = stored

func save() -> bool:
	var file := ConfigFile.new()
	file.set_value("harbor", "format", FORMAT)
	file.set_value("harbor", "stage", stage)
	# A failed or interrupted write must keep the previous checkpoint.
	var temp := file_path + ".tmp"
	if file.save(temp) != OK:
		return false
	return DirAccess.rename_absolute(ProjectSettings.globalize_path(temp),
			ProjectSettings.globalize_path(file_path)) == OK

func has_shotgun() -> bool:
	return stage >= UPPER_PIER

func spawn() -> Vector2:
	match stage:
		SUPPLY: return PIER_CHECKPOINT
		UPPER_PIER: return SUPPLY_POINT
		BEACON: return UPPER_CHECKPOINT
		COMPLETE: return BEACON_POINT
	return START

func update(pos: Vector2, grounded: bool, apex: int) -> bool:
	var before := stage
	match stage:
		FIRST_FLIGHT:
			if not grounded and apex >= 22:
				stage = LOW_PIER
		LOW_PIER:
			if grounded and pos.x >= FIRST_WALL-8.0 and pos.x < UPPER_WALL and absf(pos.y-PIER_CHECKPOINT.y) < 2.0:
				stage = SUPPLY
		SUPPLY:
			if grounded and absf(pos.x-SUPPLY_POINT.x) <= 92.0 and absf(pos.y-SUPPLY_POINT.y) < 2.0:
				stage = UPPER_PIER
		UPPER_PIER:
			if grounded and pos.x >= UPPER_WALL+8.0 and pos.y <= UPPER_CHECKPOINT.y+2.0:
				stage = BEACON
		BEACON:
			if grounded and absf(pos.x-BEACON_POINT.x) <= 112.0 and absf(pos.y-BEACON_POINT.y) < 2.0:
				stage = COMPLETE
	return before != stage

func title() -> String:
	return ["FIRST FLIGHT", "REACH THE LOW PIER", "OPEN THE SUPPLY CASE",
			"CLIMB THE HIGH PIER", "LIGHT THE BEACON", "BEACON LIT"][stage]

func instruction(pos: Vector2) -> String:
	match stage:
		FIRST_FLIGHT: return "Pull the pad down. Recoil sends you up."
		LOW_PIER: return "Follow the gold dot onto the step."
		SUPPLY: return "Land near the brass case on the pier."
		UPPER_PIER:
			if pos.x > UPPER_WALL - 35.0 and pos.y > UPPER_CHECKPOINT.y:
				return "Back off, then jump above the wall."
			return "Use the shotgun to rise onto the high pier."
		BEACON: return "Climb the terraces and land by the lamp."
		COMPLETE: return "The harbor has its light again."
	return ""

func objective_position() -> Vector2:
	match stage:
		LOW_PIER: return PIER_CHECKPOINT
		SUPPLY: return SUPPLY_POINT
		UPPER_PIER: return UPPER_CHECKPOINT
		BEACON: return BEACON_POINT
	return Vector2.INF

func guide(pos: Vector2, grounded: bool, recoil: float) -> Vector2:
	if not grounded or stage == COMPLETE:
		return Vector2.ZERO
	if stage == FIRST_FLIGHT:
		return Vector2.DOWN
	if stage == SUPPLY and pos.y <= SUPPLY_POINT.y+2.0:
		return _ground_hint(SUPPLY_POINT.x-pos.x, recoil)
	if stage == BEACON and pos.y <= BEACON_POINT.y+2.0:
		return _ground_hint(BEACON_POINT.x-pos.x, recoil)
	var ledge := FIRST_WALL
	var height := PIER_CHECKPOINT.y
	# If the player falls to an earlier terrace, point at its next step again.
	if stage >= UPPER_PIER and pos.y <= PIER_CHECKPOINT.y+2.0:
		ledge = UPPER_WALL
		height = UPPER_CHECKPOINT.y
	if stage == BEACON and pos.y <= UPPER_CHECKPOINT.y+2.0:
		ledge = ROOF_WALL if pos.y >= UPPER_CHECKPOINT.y-2.0 else BEACON_WALL
		height = 592.0 if ledge == ROOF_WALL else BEACON_POINT.y
	if pos.y > height+6.0 and pos.x < ledge:
		var gap := ledge-pos.x
		var approach := 63.0 if ledge == FIRST_WALL and recoil < 800.0 else 92.0
		if gap < 34.0 or gap > approach + 44.0:
			return _ground_hint(gap-approach, recoil)
		var tilt := clampf((gap-7.0)/(174.0 if ledge == FIRST_WALL and recoil < 800.0 else 284.0), 0.13, 0.59)
		return Vector2(-tilt, 1.0).normalized()
	return Vector2.ZERO

func _ground_hint(distance: float, recoil: float) -> Vector2:
	# A shallow upward shot aims recoil down into the floor for fine positioning.
	# This dot is instruction only. It never supplies firing input.
	var x := clampf(distance * -log(T.GROUND_DRAG) / recoil, -1.0, 1.0)
	return Vector2(-x, -sqrt(maxf(0.0, 1.0-x*x)))
