extends Node2D
## Terrain is editable in the scene. Use the same X-then-Y collision ordering
## as the raylib prototype, with positions in Godot's Y-down world.
const T = preload("res://src/tuning.gd")
const ROOM := Vector2(2464, 928)
const START := Vector2(220, 816)
const TARGETS := [Rect2(627, 784, 34, 48), Rect2(1020, 656, 34, 48), Rect2(1563, 528, 34, 48)]
@onready var terrain: TileMapLayer = $Terrain
var target_pulse: Array[float] = [0.0, 0.0, 0.0]
var hits := 0

func solid_at(point: Vector2) -> bool:
	return terrain.get_cell_source_id(terrain.local_to_map(point)) != -1

func move_box(pos: Vector2, velocity: Vector2, dt: float) -> Array:
	var grounded := false
	if dt <= 0.0:
		return [pos, velocity, supported(pos)]
	if velocity.x != 0.0:
		pos.x += velocity.x * dt
		var box := Rect2(pos - Vector2.ONE*T.HALF, Vector2.ONE*T.HALF*2.0)
		var fixed_x := pos.x
		for tile in _solids(box):
			if box.intersects(tile):
				if velocity.x > 0.0:
					fixed_x = minf(fixed_x, tile.position.x-T.HALF)
				else:
					fixed_x = maxf(fixed_x, tile.end.x+T.HALF)
		if fixed_x != pos.x:
			pos.x = fixed_x
			velocity.x = 0.0
	pos.y += velocity.y * dt
	var box := Rect2(pos - Vector2.ONE*T.HALF, Vector2.ONE*T.HALF*2.0)
	var fixed_y := pos.y
	for tile in _solids(box):
		if box.intersects(tile):
			if velocity.y > 0.0:
				fixed_y = minf(fixed_y, tile.position.y-T.HALF)
				grounded = true
			elif velocity.y < 0.0:
				fixed_y = maxf(fixed_y, tile.end.y+T.HALF)
	if fixed_y != pos.y:
		pos.y = fixed_y
		velocity.y = 0.0
	if not grounded and velocity.y >= 0.0:
		var depth := _ground_gap(pos)
		if depth <= T.SNAP:
			pos.y += depth
			velocity.y = 0.0
			grounded = true
	return [pos, velocity, grounded]

func supported(pos: Vector2) -> bool:
	return _ground_gap(pos) <= T.SNAP

func _ground_gap(pos: Vector2) -> float:
	var below := Rect2(pos.x-T.HALF, pos.y+T.HALF, T.HALF*2.0, T.SNAP+0.01)
	var depth := INF
	for tile in _solids(below):
		var gap := tile.position.y-below.position.y
		if gap >= 0.0 and tile.position.x < below.end.x and tile.end.x > below.position.x:
			depth = minf(depth, gap)
	return depth

func _solids(rect: Rect2) -> Array[Rect2]:
	var out: Array[Rect2] = []
	var low := terrain.local_to_map(rect.position)
	var high := terrain.local_to_map(rect.end - Vector2(0.001, 0.001))
	for y in range(low.y, high.y+1):
		for x in range(low.x, high.x+1):
			var cell := Vector2i(x, y)
			if terrain.get_cell_source_id(cell) >= 0:
				out.append(Rect2(terrain.map_to_local(cell)-Vector2.ONE*T.TILE*0.5, Vector2.ONE*T.TILE))
	return out

func hit_at(point: Vector2) -> bool:
	for i in range(TARGETS.size()):
		if TARGETS[i].has_point(point):
			target_pulse[i] = 0.22
			hits += 1
			queue_redraw()
			return true
	return solid_at(point)

func fade(dt: float) -> void:
	var dirty := false
	for i in range(target_pulse.size()):
		if target_pulse[i] > 0.0:
			dirty = true
			target_pulse[i] = maxf(target_pulse[i]-dt, 0.0)
	if dirty:
		queue_redraw()

func _draw() -> void:
	draw_rect(Rect2(-2400, -1700, 7500, 4500), Color("#0a1425"))
	draw_rect(Rect2(0, 0, ROOM.x, ROOM.y), Color("#0e1c30"))
	for x in range(0, int(ROOM.x)+1, 64):
		draw_line(Vector2(x, 0), Vector2(x, 832), Color("#152841"))
	for y in range(0, 832, 64):
		draw_line(Vector2(0, y), Vector2(ROOM.x, y), Color("#152841"))
	for x in range(160, int(ROOM.x), 320):
		draw_rect(Rect2(x, 180, 18, 650), Color("#122138"))
		draw_rect(Rect2(x+18, 194, 62, 4), Color("#223d56"))
		draw_rect(Rect2(x+20, 200, 28, 2), Color("#34506b"))
	_floor_marks(125, 832, "01  /  LAUNCH LINE", "Aim down to rise")
	_floor_marks(446, 768, "02  /  LOW STEP", "+64 px")
	_floor_marks(840, 704, "03  /  HIGH STEP", "+128 px")
	_floor_marks(1158, 608, "04  /  SHOTGUN LEDGE", "+224 px")
	_floor_marks(1509, 512, "05  /  UPPER DECK", "+320 px")
	_floor_marks(1830, 416, "06  /  APEX", "+416 px")
	for i in range(TARGETS.size()):
		var rect: Rect2 = TARGETS[i]
		draw_line(rect.get_center()-Vector2(0, -16), Vector2(rect.get_center().x, rect.end.y+14), Color("#45627c"), 3)
		draw_rect(rect, Color("#fff0a5") if target_pulse[i] > 0.0 else Color("#ab6261"))
		draw_rect(rect.grow(-4), Color("#172943"))
		draw_circle(rect.get_center(), 10.0, Color("#f2ac6c"))
		draw_circle(rect.get_center(), 4.0, Color("#f9e2ac"))

func _floor_marks(x: float, y: float, title: String, subtitle: String) -> void:
	var font := ThemeDB.fallback_font
	draw_rect(Rect2(x, y-96, 184, 43), Color("#142840"))
	draw_rect(Rect2(x, y-96, 3, 43), Color("#5ecbc3"))
	draw_string(font, Vector2(x+12, y-77), title, HORIZONTAL_ALIGNMENT_LEFT, -1, 13, Color("#c8e7ef"))
	draw_string(font, Vector2(x+12, y-61), subtitle, HORIZONTAL_ALIGNMENT_LEFT, -1, 12, Color("#7596b0"))
