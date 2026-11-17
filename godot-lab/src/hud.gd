extends Control
const T = preload("res://src/tuning.gd")
const Pixel = preload("res://src/pixel_ink.gd")
const INK := Color("#dfeffa")
const SUB := Color("#b0aec7")
const MINT := Color("#c1d9ec")
const GOLD := Color("#ffd08a")

var lab: Node2D

func _ready() -> void:
	texture_filter = CanvasItem.TEXTURE_FILTER_NEAREST

func pad_center() -> Vector2:
	return size - T.PAD_MARGIN - Vector2.ONE*T.PAD_RADIUS

func action_at(point: Vector2) -> String:
	if lab.menu == "completed" or lab.menu == "restart_route":
		for i in range(2):
			if _story_button(i).has_point(point):
				return ["resume", "confirm_restart"][i] if lab.menu == "completed" else ["cancel_restart", "restart_route"][i]
		return ""
	if lab.menu == "pause":
		for i in range(4):
			if _pause_button(i).has_point(point):
				return ["resume", "restart", "shake", "knob"][i]
		if _pause_mode().has_point(point):
			return "toggle_route"
		if lab.route_enabled and _pause_new().has_point(point):
			return "confirm_restart"
		return ""
	if lab.menu == "guns":
		for i in range(2):
			if _gun_card(i).has_point(point) and i < lab.guns.equipped_count:
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

func _pause_mode() -> Rect2:
	return Rect2(_panel().position+Vector2(36, 419), Vector2(280, 38))

func _pause_new() -> Rect2:
	return Rect2(_panel().position+Vector2(336, 419), Vector2(280, 38))

func _story_button(i: int) -> Rect2:
	return Rect2(_panel().position+Vector2(36, 262+i*72), Vector2(580, 56))

func _gun_card(i: int) -> Rect2:
	return Rect2(_panel().position+Vector2(36+i*294, 149), Vector2(286, 198))

func _gun_close() -> Rect2:
	return Rect2(_panel().position+Vector2(36, 370), Vector2(580, 60))

func _draw() -> void:
	if lab == null:
		return
	var slot: int = lab.guns.selected
	var spec: Dictionary = T.GUNS[slot]
	var count: int = lab.guns.ammo[slot]
	_panel_box(Rect2(26, 25, 205, 74), Color("#141722dd"), Color("#52516c"))
	_text(Vector2(41, 55), spec.name, 22, INK)
	for i in range(int(spec.ammo)):
		draw_rect(Rect2(41+i*23, 72, 15, 6), GOLD if i<count else Color("#514d64"))
	_panel_box(Rect2(size.x-258, 32, 127, 37), Color("#141722bf"), Color("#4d4b64"))
	_center(Rect2(size.x-258,32,127,37), "APEX %d px" % lab.flight_peak, 14, SUB)
	if lab.menu == "":
		_button(_pause_corner(), "II", true)
		_button(_left(0), "BRACE", lab.touch.brace_id != -999)
		_button(_left(1), "RELOAD", lab.guns.queued or lab.guns.reload_left > 0.0)
		_button(_left(2), "GUNS")
		_pad()
		if lab.route_enabled:
			_objective()

		var status := "AIM DOWN TO RISE" if lab.guns.shot_count == 0 and not lab.route_enabled else ""
		if lab.guns.queued:
			status = "RELOAD QUEUED   /   LAND TO RELOAD"
		elif lab.guns.reload_left > 0.0:
			status = "RELOADING   /   %.2f s" % lab.guns.reload_left
		elif lab.touch.blocked:
			status = "RELEASE THE PAD TO FIRE AGAIN"
		elif lab.touch.pinch:
			status = "PINCH TO ZOOM   /   %.2fx" % lab.zoom_level
		if status.is_empty() and lab.notice_left > 0.0:
			status = lab.notice
		if not status.is_empty():
			var note := Rect2(size.x*0.5-263, size.y-47, 526, 32)
			_panel_box(note, Color("#171a29cc"), Color("#635d78"))
			_center(note, status, 15, GOLD if lab.guns.queued or lab.guns.reload_left > 0.0 else SUB)
		return

	draw_rect(Rect2(Vector2.ZERO, size), Color("#0d1020ba"))
	var panel := _panel()
	_panel_box(panel, Color("#232238"), Color("#69617c"))
	draw_rect(Rect2(panel.position, Vector2(panel.size.x, 4)), MINT)
	if lab.menu == "completed" or lab.menu == "restart_route":
		_story_panel(panel)
	elif lab.menu == "pause":
		_text(panel.position+Vector2(36, 57), "MOONLIT HARBOR" if lab.route_enabled else "FREE PRACTICE", 34, INK)
		_text(panel.position+Vector2(36, 97), "Back: resume   /   Pinch outside the buttons to zoom", 18, SUB)
		_button(_pause_button(0), "RESUME", true)
		_button(_pause_button(1), "RETRY CHECKPOINT" if lab.route_enabled else "RESET RANGE")
		_button(_pause_button(2), "CAMERA SHAKE  " + ("ON" if lab.shake_enabled else "OFF"))
		_button(_pause_button(3), "CONTROL  " + ("AIM PAD" if lab.knob else "TOUCH CIRCLE"))
		_button(_pause_mode(), "FREE PRACTICE" if lab.route_enabled else "HARBOR ROUTE")
		if lab.route_enabled:
			_button(_pause_new(), "REPLAY ROUTE")
	else:
		_text(panel.position+Vector2(36, 57), "CHOOSE YOUR RECOIL", 34, INK)
		_text(panel.position+Vector2(36, 97), "World time: 8%   /   Ammo stays with each gun", 18, SUB)
		for i in range(2):
			var card := _gun_card(i)
			var gun: Dictionary = T.GUNS[i]
			_panel_box(card, Color("#373654") if i == slot else Color("#292a42"), MINT if i == slot else Color("#55526d"))
			_text(card.position+Vector2(19, 44), gun.name, 26, INK)
			if i >= lab.guns.equipped_count:
				_text(card.position+Vector2(19, 98), "LOCKED", 26, SUB)
				_text(card.position+Vector2(19, 141), "From the supply case", 17, SUB)
				continue
			_text(card.position+Vector2(19, 101), "%d / %d" % [lab.guns.ammo[i], gun.ammo], 38, GOLD if i == 1 else MINT)
			_text(card.position+Vector2(19, 145), "RECOIL  %d" % gun.recoil, 19, SUB)
			_text(card.position+Vector2(19, 176), "EQUIPPED" if i == slot else "TAP TO EQUIP", 14, SUB)
		_button(_gun_close(), "BACK TO HARBOR" if lab.route_enabled else "BACK TO RANGE")

func _objective() -> void:
	var panel := Rect2(size.x*0.5-243, 24, 486, 66)
	_panel_box(panel, Color("#171b30db"), Color("#897888"))
	var progress: int = mini(lab.route.stage+1, 5)
	_text(panel.position+Vector2(15,24), "HARBOR %d/5  /  %s" % [progress, lab.route.title()], 16, GOLD, 456)
	_text(panel.position+Vector2(15,50), lab.route.instruction(lab.player), 17, INK, 456)

func _story_panel(panel: Rect2) -> void:
	var at := panel.position+Vector2(36, 57)
	if lab.menu == "completed":
		_text(at, "THE BEACON IS LIT", 34, GOLD)
		_text(at+Vector2(0,56), "LUMINA", 19, GOLD)
		_text(at+Vector2(0,90), "That's the harbor, safe for another night.", 22, INK, 580)
		_text(at+Vector2(0,125), "Next time, I'll show you the road beyond it.", 20, SUB, 580)
		_button(_story_button(0), "KEEP EXPLORING", true)
		_button(_story_button(1), "REPLAY THE ROUTE")
	else:
		_text(at, "REPLAY THE ROUTE?", 34, INK)
		_text(at+Vector2(0,56), "This restarts the harbor and locks the shotgun again.", 20, SUB, 580)
		_button(_story_button(0), "CANCEL", true)
		_button(_story_button(1), "START AT THE WATERFRONT")

func _pad() -> void:
	var center := pad_center()
	var touch: RefCounted = lab.touch
	var tip: Vector2 = touch.pad_offset.limit_length(T.PAD_RADIUS-T.KNOB_RADIUS)
	var color := GOLD if touch.blocked else (MINT if tip.length() >= T.DEAD_ZONE else Color("#9eb6c999"))
	Pixel.stamp(self, Pixel.PAD_FILL, center, T.PAD_RADIUS, Color("#11172469"))
	Pixel.stamp(self, Pixel.PAD_RING, center, T.PAD_RADIUS, color)
	Pixel.stamp(self, Pixel.DEAD_RING, center, T.DEAD_ZONE, Color("#9eb6c960"))
	for a in [0.0, PI/2.0, PI, 3.0*PI/2.0]:
		var v := Vector2.from_angle(a)
		Pixel.line(self, center+v*(T.PAD_RADIUS-12.0), center+v*(T.PAD_RADIUS-3.0), color)
	if tip.length() >= T.DEAD_ZONE:
		Pixel.line(self, center, center+tip, color)
	if lab.route_enabled and touch.pad_id < 0 and not touch.pinch and not touch.blocked:
		var hint: Vector2 = lab.route.guide(lab.player, lab.grounded, T.GUNS[lab.guns.selected].recoil)
		if not hint.is_zero_approx():
			var marker := center + hint * 65.0
			Pixel.stamp(self, Pixel.HINT_RING, marker, 12.0, Color("#ffd08acc"))
			Pixel.line(self, center + hint*30.0, marker-hint*13.0, Color("#ffd08a6e"))
	if lab.knob:
		Pixel.stamp(self, Pixel.KNOB_FILL, center+tip, T.KNOB_RADIUS, Color("#3b4d6490"))
		Pixel.stamp(self, Pixel.KNOB_RING, center+tip, T.KNOB_RADIUS, color)
	_center(Rect2(center.x-123, center.y+108, 246, 24), "RELEASE TO FIRE" if touch.blocked else ("AIM PAD" if lab.knob else "TOUCH CIRCLE"), 16, color)

func _panel_box(rect: Rect2, fill: Color, border: Color) -> void:
	Pixel.panel(self, rect, fill, border)

func _button(rect: Rect2, label: String, active := false) -> void:
	_panel_box(rect, Color("#42405bdc") if active else Color("#1b2033d4"), MINT if active else Color("#68627d"))
	if label == "II":
		var p := rect.get_center().round()
		draw_rect(Rect2(p-Vector2(9,10),Vector2(6,20)),MINT)
		draw_rect(Rect2(p+Vector2(3,-10),Vector2(6,20)),MINT)
	else:
		_center(rect, label, 20, MINT if active else INK)

func _center(rect: Rect2, label: String, font_size: int, color: Color) -> void:
	var width := Pixel.measure(label)
	var scale := _fit(width, font_size, rect.size.x-16)
	Pixel.draw_label(self, rect.get_center()-Vector2(width,7)*scale*0.5, label, scale, color)

func _fit(width: int, font_size: int, available: float) -> int:
	var requested := maxi(1, roundi(font_size/7.0))
	return clampi(floori(available/maxi(width,1)),1,requested)

func _text(pos: Vector2, text: String, font_size: int, color: Color, wrap_width := 640.0) -> void:
	var scale := _fit(Pixel.measure(text), font_size, wrap_width)
	Pixel.draw_label(self, pos-Vector2(0,7*scale), text, scale, color)
