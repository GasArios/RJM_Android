extends SceneTree

const Lab = preload("res://scenes/lab.tscn")
const Route = preload("res://src/harbor_route.gd")
const T = preload("res://src/tuning.gd")
const SAVE = "user://harbor-route-ci.cfg"

var errors := 0

func _initialize() -> void:
	call_deferred("_run")

func check(assertion: bool, message: String) -> void:
	if not assertion:
		errors += 1
		push_error(message)

func _run() -> void:
	var absolute := ProjectSettings.globalize_path(SAVE)
	DirAccess.remove_absolute(absolute)
	var lab := Lab.instantiate()
	lab.route.file_path = SAVE
	root.add_child(lab)
	lab.set_route_enabled(true, false)
	lab.set_physics_process(false)
	await process_frame
	for point in [Route.START, Route.PIER_CHECKPOINT, Route.SUPPLY_POINT, Route.UPPER_CHECKPOINT, Route.BEACON_POINT]:
		check(lab.field.supported(point), "Checkpoint is not supported: %s" % point)

	check(lab.guns.equipped_count == 1 and not lab.guns.select(1), "Shotgun unlocked at spawn")
	check(not lab.route.update(Route.BEACON_POINT, true, 0), "Beacon could skip required stages")
	lab.guns.ammo[0] = 0
	check(lab.guns.try_fire() == -1 and lab.guns.queued, "Empty equipped revolver did not queue reload")
	lab.guns.advance(3.0, false)
	check(lab.guns.ammo == [0, 0], "Locked shotgun or airborne reload leaked ammunition")
	lab.guns.advance(0.0, true)
	check(lab.guns.advance(0.41, true) and lab.guns.ammo == [6, 0], "Revolver-only set reloaded the wrong weapons")

	# Follow the gold aim dot with normal touch input. Never set player position,
	# skip a waypoint, grant the shotgun directly, or apply a scripted impulse.
	var stages := {Route.FIRST_FLIGHT: true}
	var taps := 0
	while taps < 35 and lab.route.stage != Route.COMPLETE:
		var direction: Vector2 = lab.route.guide(lab.player, lab.grounded, T.GUNS[lab.guns.selected].recoil)
		if direction.is_zero_approx():
			break
		if lab.guns.ammo[lab.guns.selected] == 0:
			lab._action("reload", 12)
			for tick in range(90):
				lab.advance(1.0/60.0)
		var before: Vector2 = lab.player
		lab.pointer_down(4, lab.hud.pad_center()+direction*65.0)
		lab.pointer_up(4)
		for tick in range(150):
			lab.advance(1.0/60.0)
		taps += 1
		stages[lab.route.stage] = true
		print("RJM_HARBOR walkthrough tap=%d stage=%d from=%s to=%s angle=%.2f" %
				[taps, lab.route.stage, before, lab.player, direction.angle()])
	check(lab.route.stage == Route.COMPLETE and lab.menu == "completed", "Could not light the beacon through normal recoil and touch")
	check(stages.size() == 6, "The route skipped a required milestone")
	check(lab.guns.equipped_count == 2 and lab.guns.selected == 1, "Supply case did not equip unlocked shotgun")
	var recovery := Route.new()
	recovery.stage = Route.BEACON
	var below_low := recovery.guide(Vector2(220, 816), true, 950.0)
	var below_high := recovery.guide(Vector2(800, 784), true, 950.0)
	check(below_low.y > 0.0 and below_high.y > 0.0, "Route guide did not help the player recover from a lower pier")

	var restored := Route.new()
	restored.file_path = SAVE
	restored.load_save()
	check(restored.stage == Route.COMPLETE and restored.spawn() == Route.BEACON_POINT, "Completed route did not survive reload")
	lab.route = restored
	lab.set_route_enabled(false, false)
	check(lab.field.terrain.get_used_cells().size() == 320, "Free practice map was changed")
	lab.set_route_enabled(true, false)
	lab._action("restart", 4)
	check(lab.player == Route.BEACON_POINT and lab.guns.equipped_count == 2, "Retry lost the saved unlock")
	lab._action("restart_route", 4)
	check(lab.player == Route.START and lab.guns.equipped_count == 1 and lab.route.stage == Route.FIRST_FLIGHT,
			"Replay did not restore the starting route")
	restored.load_save()
	check(restored.stage == Route.FIRST_FLIGHT, "Replay did not replace the saved progress")
	DirAccess.remove_absolute(absolute)
	lab.queue_free()
	if errors == 0:
		print("PASS Harbor complete through normal touch, checkpoint/replay, shotgun lock and old practice")
	quit(0 if errors == 0 else 1)
