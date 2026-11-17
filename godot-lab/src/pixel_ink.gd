extends RefCounted
## Pixel rendering only. Callers keep true positions for physics and hit tests.
const ART = preload("res://assets/pixel-ui.svg")
const GLYPHS = preload("res://src/pixel_glyphs.gd")
const PAD_FILL := Rect2(0, 32, 100, 100)
const PAD_RING := Rect2(100, 32, 100, 100)
const KNOB_FILL := Rect2(200, 32, 28, 28)
const KNOB_RING := Rect2(200, 60, 28, 28)
const DEAD_RING := Rect2(228, 32, 18, 18)
const HINT_RING := Rect2(228, 50, 12, 12)

static func stamp(canvas: CanvasItem, region: Rect2, center: Vector2, radius: float, color: Color) -> void:
	var side := radius * 2.0
	canvas.draw_texture_rect_region(ART, Rect2(center.round()-Vector2.ONE*radius, Vector2.ONE*side), region, color)

static func line(canvas: CanvasItem, from: Vector2, to: Vector2, color: Color, stroke := 1) -> void:
	# Rasterize on the same two-world-unit grid as the character and tiles.
	var a := Vector2i((from*0.5).round())
	var b := Vector2i((to*0.5).round())
	var d := (b-a).abs()
	var direction := (b-a).sign()
	var error := d.x-d.y
	var block := Vector2.ONE * stroke * 2.0
	var offset := Vector2.ONE * float(stroke-1)
	while true:
		canvas.draw_rect(Rect2(Vector2(a)*2.0-offset, block), color)
		if a == b:
			break
		var e := error*2
		if e > -d.y:
			error -= d.y
			a.x += direction.x
		if e < d.x:
			error += d.x
			a.y += direction.y

static func panel(canvas: CanvasItem, bounds: Rect2, fill: Color, edge: Color) -> void:
	var at := bounds.position.round()
	var size := bounds.size.round()
	canvas.draw_rect(Rect2(at+Vector2(4,0),Vector2(size.x-8,2)),edge)
	canvas.draw_rect(Rect2(at+Vector2(4,size.y-2),Vector2(size.x-8,2)),edge.darkened(0.25))
	for x in [0.0, size.x-2.0]:
		canvas.draw_rect(Rect2(at+Vector2(x,4),Vector2(2,size.y-8)),edge)
	for x in [2.0, size.x-4.0]:
		for y in [2.0,size.y-4.0]:
			canvas.draw_rect(Rect2(at+Vector2(x,y),Vector2(2,2)),edge)
	canvas.draw_rect(Rect2(at+Vector2(2,4),Vector2(size.x-4,size.y-8)),fill)
	canvas.draw_rect(Rect2(at+Vector2(4,2),Vector2(size.x-8,2)),fill)
	canvas.draw_rect(Rect2(at+Vector2(4,size.y-4),Vector2(size.x-8,2)),fill)

static func advance_of(code: int) -> int:
	return GLYPHS.ADVANCE.unicode_at(code-32)-48

static func glyph(code: int) -> int:
	return code if code >= 32 and code < 96 and advance_of(code) > 0 else 63

static func measure(label: String) -> int:
	var width := 0
	for c in label.to_upper():
		width += advance_of(glyph(c.unicode_at(0)))
	return maxi(0, width-1)

static func draw_label(canvas: CanvasItem, at: Vector2, label: String, scale: int, color: Color) -> void:
	at = at.round()
	for c in label.to_upper():
		var code := glyph(c.unicode_at(0))
		var slot := code-32
		if code != 32:
			var source := Rect2((slot%16)*6, floori(float(slot)/16)*8, 6, 7)
			canvas.draw_texture_rect_region(ART, Rect2(at,source.size*scale),source,color)
		at.x += advance_of(code)*scale
