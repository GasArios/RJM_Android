extends Node2D
## Backdrop strips are visual only. Their displacement is a fraction of the
## camera's travel; all terrain and the target hit boxes stay in world space.
const SKY = preload("res://assets/harbor/sky.svg")
const CLOUDS = preload("res://assets/harbor/clouds.svg")
const HILLS = preload("res://assets/harbor/hills.svg")
const TOWN = preload("res://assets/harbor/town.svg")

var last_left_top := Vector2(INF, INF)

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

func _process(_delta: float) -> void:
	var camera := get_viewport().get_camera_2d()
	if camera == null:
		return
	var left_top := (camera.position - get_viewport_rect().size / (camera.zoom * 2.0)).floor()
	if left_top != last_left_top:
		last_left_top = left_top
		queue_redraw()

func _draw() -> void:
	var corner := last_left_top if last_left_top.is_finite() else Vector2.ZERO
	draw_rect(Rect2(-2500, -2000, 8000, 5000), Color("#0b1328"))
	_strip(SKY, Vector2(-300, -495), corner, Vector2(0.78, 0.23))
	_strip(CLOUDS, Vector2(-190, -261), corner, Vector2(0.66, 0.11))
	_strip(HILLS, Vector2(-192, 201), corner, Vector2(0.52, 0.05))
	_strip(TOWN, Vector2(-252, 146), corner, Vector2(0.34, 0.02))

func _strip(tex: Texture2D, base: Vector2, corner: Vector2, parallax: Vector2) -> void:
	var shift := (Vector2(corner.x, corner.y-272.0)*parallax).snapped(Vector2(2, 2))
	draw_texture_rect(tex, Rect2(base+shift, tex.get_size()*2.0), false)
