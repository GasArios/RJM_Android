extends Node2D
## Terrain is editable in the scene. Use the same X-then-Y collision ordering
## as the raylib prototype, with positions in Godot's Y-down world.
const T = preload("res://src/tuning.gd")
const PROPS = preload("res://assets/harbor/props.svg")
const ROOM := Vector2(2464, 928)
const START := Vector2(220, 816)
const TARGETS := [Rect2(627, 784, 34, 48), Rect2(1020, 656, 34, 48), Rect2(1563, 528, 34, 48)]
@onready var terrain: TileMapLayer = $Terrain
@onready var waypoints: Node2D = $Waypoints
var route_enabled := false
var target_pulse: Array[float] = [0.0, 0.0, 0.0]
var hits := 0

func set_route(enabled: bool, progress: RefCounted) -> void:
	route_enabled = enabled
	$Terrain.visible = not enabled
	$RouteTerrain.visible = enabled
	terrain = $RouteTerrain if enabled else $Terrain
	waypoints.route = progress
	waypoints.visible = enabled
	waypoints.queue_redraw()
	queue_redraw()

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
	if route_enabled:
		return solid_at(point)
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
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST
	if route_enabled:
		for lamp in [Vector2(91,832), Vector2(279,832), Vector2(437,800),
				Vector2(779,800), Vector2(1090,672), Vector2(1321,672),
				Vector2(1652,608), Vector2(1936,544), Vector2(2320,544)]:
			_prop(lamp - Vector2(16,64), 0)
		for crate in [Vector2(129,832), Vector2(473,800), Vector2(710,800),
				Vector2(1160,672), Vector2(1525,608), Vector2(2028,544)]:
			_prop(crate - Vector2(16,64), 1)
			_prop(crate + Vector2(12,-64), 2)
		_banner(Vector2(405, 805), Color("#67455e"))
		_banner(Vector2(1045, 682), Color("#51526e"))
		_banner(Vector2(2210, 555), Color("#6f4a60"))
		return
	# Props below are scenery. Only the tile cells and unchanged TARGETS hit.
	for lamp in [Vector2(91, 832), Vector2(384, 832), Vector2(558, 768),
			Vector2(1111, 704), Vector2(1288, 608), Vector2(1694, 512), Vector2(2012, 416),
			Vector2(2310, 320), Vector2(2350, 832)]:
		_prop(lamp - Vector2(16,64), 0)
	for crate in [Vector2(305,832), Vector2(339,832), Vector2(703,832), Vector2(941,704),
			Vector2(1195,608), Vector2(1640,576), Vector2(1910,416), Vector2(2253,320)]:
		_prop(crate - Vector2(16,64), 1)
		_prop(crate + Vector2(12,-64), 2)
	for post in [Vector2(167,832), Vector2(768,832), Vector2(1471,832), Vector2(2107,832)]:
		_prop(post-Vector2(16,64), 4)
	# A few muted flags make the near roofs read as a harbor. Keep them off the edge line.
	_banner(Vector2(530, 775), Color("#67455e"))
	_banner(Vector2(878, 708), Color("#51526e"))
	_banner(Vector2(1218, 615), Color("#6f4a60"))
	_banner(Vector2(1606, 519), Color("#585675"))
	for i in range(TARGETS.size()):
		var rect: Rect2 = TARGETS[i]
		_prop(Vector2(rect.position.x+1,rect.position.y), 3)
		if target_pulse[i] > 0.0:
			var c := Color("#ffdfb3")
			c.a = minf(target_pulse[i] * 2.4, 0.48)
			draw_rect(Rect2(rect.position+Vector2(9,14), Vector2(16,23)), c)

func _prop(at: Vector2, index: int) -> void:
	draw_texture_rect_region(PROPS, Rect2(at, Vector2(32,64)), Rect2(index*16,0,16,32))

func _banner(at: Vector2, cloth: Color) -> void:
	draw_rect(Rect2(at, Vector2(26,2)), Color("#76697c"))
	draw_colored_polygon(PackedVector2Array([at+Vector2(3,2), at+Vector2(23,2),
		at+Vector2(23,36), at+Vector2(13,30), at+Vector2(3,36)]), cloth)
	draw_rect(Rect2(at+Vector2(12,11), Vector2(3,13)), Color("#a18786"))
	draw_rect(Rect2(at+Vector2(8,15), Vector2(11,3)), Color("#a18786"))
