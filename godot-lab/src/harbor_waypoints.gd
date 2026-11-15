extends Node2D
## Draw the route's landmarks on a two-pixel grid, independently of collision.
const Route = preload("res://src/harbor_route.gd")

var route: RefCounted
var glow_age := 0.0

func advance(delta: float) -> void:
	if not visible:
		return
	glow_age += delta
	queue_redraw()

func _draw() -> void:
	if route == null:
		return
	_checkpoint(Route.PIER_CHECKPOINT, route.stage >= Route.SUPPLY)
	_checkpoint(Route.UPPER_CHECKPOINT, route.stage >= Route.BEACON)
	_supply(Route.SUPPLY_POINT, route.has_shotgun())
	_beacon(Route.BEACON_POINT, route.stage == Route.COMPLETE)
	var point: Vector2 = route.objective_position()
	if point.is_finite():
		var lift := floorf(sin(glow_age * 3.0) * 2.0) * 2.0
		var pt := point.round() + Vector2(0, -60 + lift)
		_pixel(pt, -2, -1, 4, 12, "#ffe3a2")
		_pixel(pt, -6, 3, 12, 4, "#ffe3a2")
		_pixel(pt, 0, 3, 2, 4, "#fff5ca")

func _checkpoint(pos: Vector2, active: bool) -> void:
	var floor_at := pos + Vector2(0, 16)
	_pixel(floor_at, -10, -4, 20, 4, "#2d263f")
	_pixel(floor_at, -6, -30, 12, 26, "#44384f")
	_pixel(floor_at, -8, -34, 16, 4, "#8f7278")
	_pixel(floor_at, -4, -26, 8, 12, "#ffe0a0" if active else "#7d6670")
	if active:
		_pixel(floor_at, -2, -26, 4, 4, "#fff6bb")

func _supply(pos: Vector2, stocked: bool) -> void:
	var p := pos + Vector2(0, 16)
	_pixel(p, -24, -22, 48, 22, "#4b3650")
	_pixel(p, -22, -22, 44, 4, "#9b7277")
	_pixel(p, -22, -16, 44, 12, "#61465b")
	_pixel(p, -20, -16, 4, 16, "#bc8d68")
	_pixel(p, 16, -16, 4, 16, "#bc8d68")
	if not stocked:
		_pixel(p, -6, -18, 12, 10, "#d8a369")
		_pixel(p, -2, -16, 4, 4, "#fff1ac")
	else:
		_pixel(p, -20, -30, 40, 6, "#8f6872")
		_pixel(p, -18, -24, 36, 2, "#2b283f")

func _beacon(pos: Vector2, lit: bool) -> void:
	var p := pos + Vector2(0, 16)
	if lit:
		draw_circle(p+Vector2(0,-52), 48.0, Color("#e9aa5514"))
		draw_circle(p+Vector2(0,-52), 32.0, Color("#e9aa5521"))
	_pixel(p, -30, -4, 60, 4, "#837483")
	_pixel(p, -18, -34, 36, 30, "#454057")
	_pixel(p, -22, -38, 44, 4, "#928085")
	_pixel(p, -14, -60, 28, 22, "#514359")
	_pixel(p, -10, -58, 20, 16, "#bb7b59" if lit else "#6c566e")
	_pixel(p, -4, -56, 8, 12, "#fff0ad" if lit else "#8b6570")
	_pixel(p, -18, -64, 36, 4, "#695a70")
	_pixel(p, -10, -68, 20, 4, "#8b7280")
	_pixel(p, -4, -72, 8, 4, "#b09089")

func _pixel(origin: Vector2, x: int, y: int, w: int, h: int, color: String) -> void:
	draw_rect(Rect2(origin + Vector2(x,y), Vector2(w,h)), Color(color))
