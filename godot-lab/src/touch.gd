extends RefCounted
## Every pointer keeps the role assigned at touch down.
const T = preload("res://src/tuning.gd")

var pad_id := -999
var brace_id := -999
var pad_offset := Vector2.ZERO
var direction := Vector2.DOWN
var press_pending := false
var blocked := false
var pinch := false
var world_fingers: Dictionary = {}
var pinch_gap := 0.0

func pad_down(id: int, offset: Vector2) -> bool:
	if pad_id != -999 or pinch or offset.length() > T.PAD_RADIUS:
		return false
	pad_id = id
	pad_offset = Vector2.ZERO
	blocked = false
	pad_move(id, offset)
	return true

func pad_move(id: int, offset: Vector2) -> void:
	if id != pad_id:
		return
	var was_live := pad_offset.length() >= T.DEAD_ZONE
	pad_offset = offset
	if pad_offset.length() < T.DEAD_ZONE:
		press_pending = false
	else:
		direction = offset.normalized()
		if not was_live and not blocked:
			press_pending = true

func wants_fire() -> bool:
	var wanted := not blocked and not pinch and (press_pending or (pad_id != -999 and pad_offset.length() >= T.DEAD_ZONE))
	press_pending = false
	return wanted

func block_until_release() -> void:
	blocked = pad_id != -999
	press_pending = false

func release(id: int) -> void:
	if id == pad_id:
		pad_id = -999
		pad_offset = Vector2.ZERO
		blocked = false
	if id == brace_id:
		brace_id = -999
	world_fingers.erase(id)
	if world_fingers.is_empty():
		pinch = false
		pinch_gap = 0.0

func cancel() -> void:
	pad_id = -999
	brace_id = -999
	press_pending = false
	blocked = false
	pad_offset = Vector2.ZERO
	world_fingers.clear()
	pinch = false
	pinch_gap = 0.0

func world_down(id: int, pos: Vector2) -> void:
	if world_fingers.size() >= 2 or pad_id != -999 or brace_id != -999:
		return
	world_fingers[id] = pos
	if world_fingers.size() == 2:
		pinch = true
		pinch_gap = _gap()

func world_move(id: int, pos: Vector2) -> float:
	if not world_fingers.has(id):
		return 1.0
	world_fingers[id] = pos
	if not pinch or world_fingers.size() < 2:
		return 1.0
	var new_gap := _gap()
	var factor := new_gap / pinch_gap if new_gap >= 24.0 and pinch_gap >= 24.0 else 1.0
	pinch_gap = new_gap
	return factor

func _gap() -> float:
	var a: Vector2 = world_fingers.values()[0]
	var b: Vector2 = world_fingers.values()[1]
	return a.distance_to(b)
