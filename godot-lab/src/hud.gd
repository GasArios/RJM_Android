extends Control
const T = preload("res://src/tuning.gd")
const INK := Color("#dfeffa")
const SUB := Color("#8ca7c2")
const MINT := Color("#7ce4d1")
const GOLD := Color("#ffd08a")

var lab: Node2D

func pad_center() -> Vector2:
	return size - T.PAD_MARGIN - Vector2.ONE*T.PAD_RADIUS

func action_at(point: Vector2) -> String:
	if lab.menu == "pause":
		for i in range(4):
			if _pause_button(i).has_point(point):
				return ["resume", "restart", "shake", "knob"][i]
		return ""
	if lab.menu == "guns":
		for i in range(2):
			if _gun_card(i).has_point(point):
				return ["revolver", "shotgun"][i]
		if _gun_close().has_point(point):
			return "resume"
		return ""
	if _pause_corner().has_point(point):
		return "pause"
	if _left(0).has_point(point):
		return "brace"
	if _left(1).has_point(point):
		return "reload"
	if _left(2).has_point(point):
		return "guns"
	return ""

func _left(i: int) -> Rect2:
	return Rect2(30, size.y-282+i*74, 134, 62)

func _pause_corner() -> Rect2:
	return Rect2(size.x-114, 30, 82, 56)

func _panel() -> Rect2:
	return Rect2((size.x-652)*0.5, (size.y-472)*0.5, 652, 472)

func _pause_button(i: int) -> Rect2:
	return Rect2(_panel().position+Vector2(36, 146+i*67), Vector2(580, 56))

func _gun_card(i: int) -> Rect2:
	return Rect2(_panel().position+Vector2(36+i*294, 149), Vector2(286, 198))

func _gun_close() -> Rect2:
	return Rect2(_panel().position+Vector2(36, 370), Vector2(580, 60))

func _draw() -> void:
	if lab == null:
		return
	_panel_box(Rect2(30, 30, 376, 97), Color("#111f34ee"), Color("#314960"))
	draw_rect(Rect2(30, 30, 5, 97), MINT)
	_text(Vector2(53, 65), "RJM / GODOT LAB", 30, INK)
	_text(Vector2(55, 98), "RECOIL RANGE   /   0.1.0", 18, MINT)

	var slot: int = lab.guns.selected
	var spec: Dictionary = T.GUNS[slot]
	var count: int = lab.guns.ammo[slot]
	_text(Vector2(32, 164), spec.name, 23, INK)
	for i in range(int(spec.ammo)):
		var color := GOLD if slot == 1 else MINT
		draw_rect(Rect2(33+i*26, 181, 20, 12), color if i<count else Color("#33465d"))
	_text(Vector2(32, 217), "%d / %d  AMMO" % [count, int(spec.ammo)], 17, SUB)

	var stat_x := size.x*0.5 - 157.0
	_panel_box(Rect2(stat_x, 30, 314, 57), Color("#111f34ee"), Color("#314960"))
	_text(Vector2(stat_x+14, 64), "APEX %d px   /   HITS %d   /   %.2fx" % [lab.flight_peak, lab.field.hits, lab.zoom_level], 18, INK)
	if lab.menu == "":
		_button(_pause_corner(), "II", true)
		_button(_left(0), "BRACE", lab.touch.brace_id != -999)
		_button(_left(1), "RELOAD", lab.guns.queued or lab.guns.reload_left > 0.0)
		_button(_left(2), "GUNS")
		_pad()

		var status := "AIM DOWN TO RISE   /   HOLD OUTSIDE CENTER TO FIRE"
		if lab.guns.queued:
			status = "RELOAD QUEUED   /   LAND TO RELOAD"
		elif lab.guns.reload_left > 0.0:
			status = "RELOADING   /   %.2f s" % lab.guns.reload_left
		elif lab.touch.blocked:
			status = "RELEASE THE PAD TO FIRE AGAIN"
		elif lab.touch.pinch:
			status = "PINCH TO ZOOM   /   %.2fx" % lab.zoom_level
		var note := Rect2(size.x*0.5-296, size.y-48, 592, 33)
		_panel_box(note, Color("#111f34ee"), Color("#294760"))
		_center(note, status, 15, GOLD if lab.guns.queued or lab.guns.reload_left > 0.0 else SUB)
		return

	draw_rect(Rect2(Vector2.ZERO, size), Color("#07101dd8"))
	var panel := _panel()
	_panel_box(panel, Color("#13243a"), Color("#496983"))
	draw_rect(Rect2(panel.position, Vector2(panel.size.x, 4)), MINT)
	if lab.menu == "pause":
		_text(panel.position+Vector2(36, 57), "RANGE PAUSED", 34, INK)
		_text(panel.position+Vector2(36, 97), "Back: resume   /   Pinch outside the buttons to zoom", 18, SUB)
		_button(_pause_button(0), "RESUME", true)
		_button(_pause_button(1), "RESET RANGE")
		_button(_pause_button(2), "CAMERA SHAKE  " + ("ON" if lab.shake_enabled else "OFF"))
		_button(_pause_button(3), "CONTROL  " + ("AIM PAD" if lab.knob else "TOUCH CIRCLE"))
	else:
		_text(panel.position+Vector2(36, 57), "CHOOSE YOUR RECOIL", 34, INK)
		_text(panel.position+Vector2(36, 97), "World time: 8%   /   Ammo stays with each gun", 18, SUB)
		for i in range(2):
			var card := _gun_card(i)
			var gun: Dictionary = T.GUNS[i]
			_panel_box(card, Color("#1a324a") if i == slot else Color("#182b42"), MINT if i == slot else Color("#37536a"))
			_text(card.position+Vector2(19, 44), gun.name, 26, INK)
			_text(card.position+Vector2(19, 101), "%d / %d" % [lab.guns.ammo[i], gun.ammo], 38, GOLD if i == 1 else MINT)
			_text(card.position+Vector2(19, 145), "RECOIL  %d" % gun.recoil, 19, SUB)
			_text(card.position+Vector2(19, 176), "EQUIPPED" if i == slot else "TAP TO EQUIP", 14, SUB)
		_button(_gun_close(), "BACK TO RANGE")

func _pad() -> void:
	var center := pad_center()
	var touch: RefCounted = lab.touch
	var tip: Vector2 = touch.pad_offset.limit_length(T.PAD_RADIUS-T.KNOB_RADIUS)
	var color := GOLD if touch.blocked else MINT
	draw_circle(center, T.PAD_RADIUS, Color("#111f35c8"))
	draw_arc(center, T.PAD_RADIUS, 0, TAU, 72, color, 2.0, true)
	draw_arc(center, T.DEAD_ZONE, 0, TAU, 32, Color("#6d91a5"), 1.0, true)
	for a in [0.0, PI/2.0, PI, 3.0*PI/2.0]:
		var v := Vector2.from_angle(a)
		draw_line(center+v*(T.PAD_RADIUS-12.0), center+v*(T.PAD_RADIUS-3.0), color, 2.0, true)
	if tip.length() >= T.DEAD_ZONE:
		draw_line(center, center+tip, color, 2.0, true)
	if lab.knob:
		draw_circle(center+tip, T.KNOB_RADIUS, Color("#274d69"))
		draw_arc(center+tip, T.KNOB_RADIUS, 0, TAU, 36, color, 2.0, true)
	_center(Rect2(center.x-123, center.y+108, 246, 24), "RELEASE TO FIRE" if touch.blocked else ("AIM PAD" if lab.knob else "TOUCH CIRCLE"), 16, color)

func _panel_box(rect: Rect2, fill: Color, border: Color) -> void:
	var box := StyleBoxFlat.new()
	box.bg_color = fill
	box.border_color = border
	box.set_border_width_all(1)
	box.set_corner_radius_all(10)
	draw_style_box(box, rect)

func _button(rect: Rect2, label: String, active := false) -> void:
	_panel_box(rect, Color("#234659ef") if active else Color("#14253ce9"), MINT if active else Color("#3a5770"))
	_center(rect, label, 20, MINT if active else INK)

func _center(rect: Rect2, label: String, font_size: int, color: Color) -> void:
	var font := ThemeDB.fallback_font
	var w := font.get_string_size(label, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size).x
	_text(rect.get_center()+Vector2(-w*0.5, font_size*0.35), label, font_size, color)

func _text(pos: Vector2, text: String, font_size: int, color: Color) -> void:
	draw_string(ThemeDB.fallback_font, pos, text, HORIZONTAL_ALIGNMENT_LEFT, -1, font_size, color)
