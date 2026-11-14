extends Node2D
const T = preload("res://src/tuning.gd")
const Recoil = preload("res://src/recoil.gd")
const Loadout = preload("res://src/loadout.gd")
const Touch = preload("res://src/touch.gd")
const PREFS := "user://lab-settings.cfg"
const MOUSE_ID := -1

@onready var field: Node2D = $Field
@onready var camera: Camera2D = $Camera2D
@onready var hud: Control = $CanvasLayer/HUD
var guns := Loadout.new()
var touch := Touch.new()
var player := Vector2(220, 816)
var velocity := Vector2.ZERO
var grounded := true
var launch_y := 816.0
var flight_peak := 0
var menu := ""
var zoom_level := T.DEFAULT_ZOOM
var shake_enabled := true
var knob := true
var stop_left := 0.0
var shake_left := 0.0
var shake_time := 1.0
var shake_strength := 0.0
var visual_time := 0.0
var flash := 0.0
var bullets: Array[Dictionary] = []
var direction := Vector2.DOWN

func _ready() -> void:
	var config := ConfigFile.new()
	if config.load(PREFS) == OK:
		zoom_level = clampf(float(config.get_value("lab", "zoom", T.DEFAULT_ZOOM)), T.MIN_ZOOM, T.MAX_ZOOM)
		shake_enabled = bool(config.get_value("lab", "shake", true))
		knob = bool(config.get_value("lab", "knob", true))
	hud.lab = self
	camera.zoom = Vector2.ONE*zoom_level
	camera.position = _camera_goal()
	print("RJM_GODOT ready zoom=%.2f knob=%d" % [zoom_level, int(knob)])

func _notification(what: int) -> void:
	if not is_node_ready():
		return
	if what == NOTIFICATION_WM_GO_BACK_REQUEST:
		_back()
	elif what == NOTIFICATION_APPLICATION_PAUSED or what == NOTIFICATION_APPLICATION_FOCUS_OUT:
		set_menu("pause")

func _input(event: InputEvent) -> void:
	if event is InputEventScreenTouch:
		if event.pressed and not event.canceled:
			pointer_down(event.index, event.position)
		else:
			if event.canceled:
				touch.press_pending = false
			pointer_up(event.index)
	elif event is InputEventScreenDrag:
		pointer_move(event.index, event.position)
	elif event is InputEventMouseButton and not OS.has_feature("android"):
		if event.button_index == MOUSE_BUTTON_LEFT:
			if event.pressed:
				pointer_down(MOUSE_ID, event.position)
			else:
				pointer_up(MOUSE_ID)
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_UP:
			_update_zoom(1.1)
			_save()
		elif event.pressed and event.button_index == MOUSE_BUTTON_WHEEL_DOWN:
			_update_zoom(1.0/1.1)
			_save()
	elif event is InputEventMouseMotion and not OS.has_feature("android"):
		pointer_move(MOUSE_ID, event.position)
	elif event is InputEventKey and event.pressed and not event.echo and not OS.has_feature("android"):
		match event.keycode:
			KEY_ESCAPE: _back()
			KEY_R: _action("reload", MOUSE_ID)
			KEY_1: _action("revolver", MOUSE_ID)
			KEY_2: _action("shotgun", MOUSE_ID)

func pointer_down(id: int, point: Vector2) -> void:
	var action: String = hud.action_at(point)
	if not action.is_empty():
		_action(action, id)
	elif menu == "" and touch.pad_down(id, point-hud.pad_center()):
		pass
	elif menu != "guns" and point.distance_to(hud.pad_center()) > T.PAD_RADIUS:
		touch.world_down(id, point)

func pointer_move(id: int, point: Vector2) -> void:
	if menu == "" and id == touch.pad_id:
		touch.pad_move(id, point-hud.pad_center())
	_update_zoom(touch.world_move(id, point))

func pointer_up(id: int) -> void:
	var was_pinching: bool = touch.pinch
	touch.release(id)
	if was_pinching and not touch.pinch:
		_save()

func _back() -> void:
	set_menu("pause" if menu == "" else "")

func set_menu(next: String) -> void:
	touch.cancel()
	if menu != next:
		menu = next
		print("RJM_GODOT menu=", "world" if menu == "" else menu)
	if menu == "pause":
		_save()

func _action(action: String, id: int) -> void:
	match action:
		"brace":
			if touch.brace_id == -999 and not touch.pinch:
				touch.brace_id = id
		"reload":
			if menu == "":
				guns.request_reload()
				touch.block_until_release()
				if guns.queued:
					print("RJM_GODOT reload queued grounded=", grounded)
		"guns": set_menu("guns")
		"pause": set_menu("pause")
		"resume": set_menu("")
		"restart":
			_reset_range()
			set_menu("")
		"revolver", "shotgun":
			guns.selected = 0 if action == "revolver" else 1
			set_menu("")
			print("RJM_GODOT selected ", T.GUNS[guns.selected].name)
		"shake":
			shake_enabled = not shake_enabled
			_save()
		"knob":
			knob = not knob
			_save()
			print("RJM_GODOT knob=", int(knob))

func _reset_range() -> void:
	guns = Loadout.new()
	bullets.clear()
	field.hits = 0
	player = field.START
	velocity = Vector2.ZERO
	launch_y = player.y
	flight_peak = 0
	grounded = true
	stop_left = 0.0
	shake_left = 0.0
	flash = 0.0
	direction = Vector2.DOWN
	camera.position = _camera_goal()

func _save() -> void:
	var config := ConfigFile.new()
	config.set_value("lab", "zoom", zoom_level)
	config.set_value("lab", "knob", knob)
	config.set_value("lab", "shake", shake_enabled)
	if config.save(PREFS) != OK:
		push_warning("Lab settings could not be saved.")

func _update_zoom(factor: float) -> void:
	if is_equal_approx(factor, 1.0):
		return
	var next := clampf(zoom_level*factor, T.MIN_ZOOM, T.MAX_ZOOM)
	camera.position = player - (player-camera.position)*(zoom_level/next)
	zoom_level = next
	camera.zoom = Vector2.ONE*zoom_level

func _physics_process(delta: float) -> void:
	advance(delta)

func advance(delta: float) -> void:
	if menu == "":
		direction = touch.direction
		if touch.wants_fire():
			var fired := guns.try_fire()
			if fired >= 0:
				_fire(fired, direction)
			elif guns.queued:
				touch.block_until_release()
				print("RJM_GODOT reload queued grounded=", grounded)
	var world_delta := delta * (0.0 if menu == "pause" else (T.SLOW if menu == "guns" or touch.pinch else 1.0))
	var stop := minf(stop_left, world_delta)
	stop_left -= stop
	var dt := world_delta-stop
	if dt > 0.0:
		# Two movement steps per 60 Hz tick, matching the raylib 60 FPS path.
		for step in range(2):
			var before := grounded
			var old_y := player.y
			velocity = Recoil.forces(velocity, dt*0.5, grounded, touch.brace_id != -999)
			var result: Array = field.move_box(player, velocity, dt*0.5)
			player = result[0]
			velocity = result[1]
			grounded = result[2]
			if before and not grounded:
				launch_y = old_y
				flight_peak = 0
			if not grounded:
				flight_peak = maxi(flight_peak, roundi(launch_y-player.y))
		if player.y > field.ROOM.y+128:
			_reset_range()
		if guns.advance(dt, grounded):
			print("RJM_GODOT reloaded")
		_move_bullets(dt)
		field.fade(dt)
	if menu != "pause":
		visual_time += delta
		flash = maxf(flash-delta, 0.0)
		shake_left = maxf(shake_left-delta, 0.0)
		camera.position = camera.position.lerp(_camera_goal(), 1.0-exp(-11.0*delta))
		var amount := shake_strength*shake_left/shake_time if shake_enabled else 0.0
		camera.offset = Vector2(sin(visual_time*137.0), cos(visual_time*178.0))*amount
	elif not shake_enabled:
		camera.offset = Vector2.ZERO
	queue_redraw()
	hud.queue_redraw()

func _camera_goal() -> Vector2:
	var half := get_viewport_rect().size/(2.0*zoom_level)
	var room: Vector2 = field.ROOM
	var goal := player+Vector2(clampf(velocity.x*0.08, -115.0, 115.0), -88.0)
	goal.x = clampf(goal.x, half.x, room.x-half.x) if half.x*2.0 < room.x else room.x*0.5
	goal.y = clampf(goal.y, half.y, room.y-half.y) if half.y*2.0 < room.y else room.y*0.5
	return goal

func _fire(slot: int, aim: Vector2) -> void:
	var gun: Dictionary = T.GUNS[slot]
	velocity = Recoil.impulse(velocity, aim, gun.recoil, grounded, touch.brace_id != -999, maxf(launch_y-player.y, 0.0), gun.resist)
	var count := int(gun.count)
	for i in range(count):
		var angle := deg_to_rad(lerpf(-gun.spread*0.5, gun.spread*0.5, float(i)/float(maxi(count-1, 1))))
		var heading := aim.rotated(angle)
		bullets.append({"pos": player, "dir": heading, "speed": gun.speed, "left": gun.range, "gold": slot == 1})
	stop_left = maxf(stop_left, gun.stop)
	shake_time = float(gun.shake_time)
	shake_left = shake_time
	shake_strength = float(gun.shake)
	flash = 0.075
	print("RJM_GODOT shot ", gun.name, " ammo=", guns.ammo[slot])

func _move_bullets(dt: float) -> void:
	for i in range(bullets.size()-1, -1, -1):
		var bullet := bullets[i]
		var length := minf(bullet.left, bullet.speed*dt)
		var steps := maxi(ceili(length/6.0), 1)
		var hit := false
		for j in range(steps):
			bullet.pos += bullet.dir*(length/steps)
			if field.hit_at(bullet.pos):
				hit = true
				break
		bullet.left -= length
		if hit or bullet.left <= 0.0:
			bullets.remove_at(i)

func _draw() -> void:
	for bullet in bullets:
		var color := Color("#ffd180") if bullet.gold else Color("#fff0a3")
		draw_line(bullet.pos-bullet.dir*13.0, bullet.pos, Color("#222031"), 6.0, true)
		draw_line(bullet.pos-bullet.dir*13.0, bullet.pos, color, 3.0, true)
	draw_rect(Rect2(player-Vector2.ONE*(T.HALF-1.0)+Vector2(2, 4), Vector2.ONE*(T.HALF*2.0)), Color("#10213e"))
	draw_rect(Rect2(player-Vector2.ONE*T.HALF, Vector2.ONE*T.HALF*2.0), Color("#59b9ff"))
	draw_rect(Rect2(player-Vector2.ONE*T.HALF, Vector2.ONE*T.HALF*2.0), Color("#c3f5ff"), false, 2.0)
	draw_rect(Rect2(player+Vector2(-8, -6), Vector2(5, 7)), Color("#14364c"))
	draw_rect(Rect2(player+Vector2(2, -6), Vector2(5, 7)), Color("#14364c"))
	draw_line(player+direction*12, player+direction*30, Color("#b3dbe8"), 6.0, true)
	if menu == "" and touch.pad_offset.length() >= T.DEAD_ZONE and not touch.blocked:
		draw_line(player+direction*36, player+direction*56, Color("#ffc77f"), 2.0, true)
		var v := -direction
		var tip := player+v*58
		draw_line(player+v*21, tip, Color("#68d4c1"), 2.0, true)
		draw_line(tip, tip-v.rotated(0.5)*12, Color("#68d4c1"), 2.0, true)
		draw_line(tip, tip-v.rotated(-0.5)*12, Color("#68d4c1"), 2.0, true)
	if flash > 0.0:
		draw_circle(player+direction*33, 8.0, Color("#fff1a5"))
