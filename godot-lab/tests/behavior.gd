extends SceneTree

const T = preload("res://src/tuning.gd")
const Recoil = preload("res://src/recoil.gd")
const RecoilBody = preload("res://src/recoil_body.gd")
const Loadout = preload("res://src/loadout.gd")
const Touch = preload("res://src/touch.gd")
const Lab = preload("res://scenes/lab.tscn")

var errors := 0

func _initialize() -> void:
	call_deferred("_run")

func check(test: bool, message: String) -> void:
	if not test:
		errors += 1
		push_error(message)

func _run() -> void:
	var body := RecoilBody.new()
	var center := Vector2(220, 816)
	check(body.pose(center, true).is_equal_approx(Transform2D(0.0, center)), "Idle body is not square")
	var diagonal := Vector2(1, -1).normalized()
	body.launch(diagonal * 950.0, 0.12)
	body.advance(0.045, diagonal * 950.0, false)
	var squeeze := body.pose(center, true)
	check(squeeze.basis_xform(diagonal).length() < 0.9, "Body did not compress during shot stop")
	body.advance(0.12, diagonal * 950.0, false)
	var stretch := body.pose(center, false)
	check(stretch.basis_xform(diagonal).length() > 1.2 and stretch.basis_xform(diagonal.orthogonal()).length() < 0.9, "Body did not stretch along the actual recoil axis")
	var edge := stretch.affine_inverse() * RecoilBody.edge(stretch, -diagonal)
	check(is_equal_approx(maxf(absf(edge.x), absf(edge.y)), T.HALF), "Drawn barrel detached from the distorted body")
	body.land(800.0)
	var landing := body.pose(center, true)
	check(landing.y.y < 0.8 and landing.x.x > 1.2, "Heavy landing did not flatten the body")
	check(is_equal_approx((landing * Vector2(T.HALF, T.HALF)).y, center.y + T.HALF), "Flattened body lost contact with floor")
	for i in range(60):
		body.advance(1.0/60.0, Vector2.ZERO, true)
	check(body.pose(center, true).is_equal_approx(Transform2D(0.0, center)), "Visual body did not return to square")
	print("PASS Godot body squash, recoil stretch, attached barrel, floor alignment and recovery")

	var aim := Touch.new()
	check(aim.pad_down(12, Vector2.ZERO) and not aim.wants_fire(), "Pad center fired")
	aim.pad_move(12, Vector2(0, 150))
	check(aim.wants_fire() and aim.direction == Vector2.DOWN, "Drag outside pad lost its direction")
	aim.pad_move(12, Vector2.ZERO)
	check(not aim.wants_fire(), "Return to center did not stop fire")
	aim.pad_move(12, Vector2(0, 60))
	aim.block_until_release()
	aim.pad_move(12, Vector2.ZERO)
	aim.pad_move(12, Vector2(0, 60))
	check(not aim.wants_fire(), "Reload gate rearmed without a release")
	aim.release(12)
	check(aim.pad_offset == Vector2.ZERO and aim.pad_down(12, Vector2(0, 60)), "Pad did not recenter")
	aim.release(12)
	check(aim.wants_fire() and not aim.wants_fire(), "Short tap should fire exactly once")
	aim.world_down(3, Vector2(300, 300))
	aim.world_down(4, Vector2(700, 300))
	check(aim.pinch and aim.world_move(4, Vector2(800, 300)) > 1 and not aim.wants_fire(), "Pinch ownership broken")
	aim.release(3)
	check(aim.pinch and not aim.pad_down(5, Vector2(10, 30)), "One remaining pinch finger leaked a shot")
	aim.release(4)
	check(not aim.pinch, "Pinch gate did not clear")

	var g := Loadout.new()
	for i in range(6):
		check(g.try_fire() == 0, "Revolver did not fire")
		check(g.try_fire() == -1 if i < 5 else true, "Cooldown bypassed")
		g.advance(0.31, false)
	check(g.try_fire() == 1, "Empty revolver did not auto-switch")
	g.advance(0.36, false)
	check(g.try_fire() == 1 and g.try_fire() == -1 and g.queued, "Whole-set reload not queued")
	g.advance(3.0, false)
	check(g.queued and g.ammo == [0, 0], "Reload ran in the air")
	g.advance(0.0, true)
	check(not g.advance(0.5, true) and g.advance(0.08, true) and g.ammo == [6, 2], "Ground reload time changed")

	var lab := Lab.instantiate()
	root.add_child(lab)
	lab.set_physics_process(false)
	await process_frame
	var cells: Array[Vector2i] = lab.field.terrain.get_used_cells()
	check(cells.size() == 320, "Harbor changed the playable cells")
	cells.sort()
	var coords := PackedStringArray()
	for cell in cells:
		coords.append("%d,%d" % [cell.x, cell.y])
	check(";".join(coords).sha256_text() == "765a0c5628be14ffed8b1b9c2f64556594e7102bdded15580939347d77960b5f", "Harbor changed a collision position")
	check(lab.field.supported(lab.player), "Spawn is not grounded")
	var fall: Array = lab.field.move_box(Vector2(510, 750), Vector2(0, 300), 0.01)
	check(fall[2] and absf(fall[0].y-752.0) < 0.01, "Landing or snap on low step failed")
	var side: Array = lab.field.move_box(Vector2(47, 800), Vector2(-300, 0), 0.02)
	check(side[1].x == 0.0 and absf(side[0].x-48.0) < 0.01, "Wall collision failed")
	check(Recoil.impulse(Vector2.ZERO, Vector2.DOWN, 600, true, true, 0, 0).is_zero_approx(), "Brace did not anchor revolver")
	check(Recoil.impulse(Vector2.ZERO, Vector2.RIGHT, 600, true, false, 0, 0).x < -590, "Recoil direction wrong")
	var peaks: Array[int] = []
	for gun in range(2):
		lab._reset_range()
		lab.guns.selected = gun
		lab.touch.pad_down(8, Vector2(0, 60))
		lab.touch.release(8)
		for frame in range(110):
			lab.advance(1.0/60.0)
		check(lab.guns.shot_count == 1 and lab.grounded, "Tap did not complete a single flight")
		peaks.append(lab.flight_peak)
	check(peaks == [76, 164], "Visual deformation changed the established recoil heights")
	print("RJM_GODOT tested apex: revolver=", peaks[0], " shotgun=", peaks[1])

	lab._reset_range()
	lab.touch.pad_down(4, Vector2(0,60))
	lab.advance(1.0/60.0)
	lab.set_menu("pause")
	var stopped: Vector2 = lab.player
	var stopped_body: Transform2D = lab.body.pose(lab.player, lab.grounded)
	for i in range(30):
		lab.advance(1.0/60.0)
	check(lab.player == stopped and lab.guns.shot_count == 1, "Pause did not freeze input/physics")
	check(lab.body.pose(lab.player, lab.grounded).is_equal_approx(stopped_body), "Paused body continued to animate")
	lab.set_menu("")
	for i in range(30):
		lab.advance(1.0/60.0)
	check(lab.guns.shot_count == 1, "Pause release leaked a held shot")
	lab.queue_free()
	if errors == 0:
		print("PASS Godot aim pad, recoil, editable map collision, weapon inventory, reload and pause")
	quit(0 if errors == 0 else 1)
