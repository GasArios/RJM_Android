extends RefCounted
## Render-only squash and stretch. The physics box never reads this state.
const T = preload("res://src/tuning.gd")
const KICK_RECOVERY := 0.28
const KICK_STRETCH_RISE := 0.045
const LAND_RECOVERY := 0.24
const KICK_LIMIT := 0.30
const TRAVEL_LIMIT := 0.12

var kick_axis := Vector2.UP
var kick_strength := 0.0
var kick_age := 0.0
var windup := 0.0
var land_age := LAND_RECOVERY
var land_strength := 0.0
var travel := Vector2.ZERO

func launch(impulse: Vector2, hold: float) -> void:
	if impulse.length_squared() < 1.0:
		return
	kick_axis = impulse.normalized()
	kick_strength = clampf(impulse.length() / 3000.0, 0.0, KICK_LIMIT)
	kick_age = 0.0
	windup = hold
	land_age = LAND_RECOVERY

func land(speed: float) -> void:
	if speed < 100.0:
		return
	land_strength = clampf((speed - 100.0) / 2200.0, 0.0, 0.26)
	land_age = 0.0
	kick_strength = 0.0

func advance(dt: float, velocity: Vector2, grounded: bool) -> void:
	kick_age += dt
	land_age += dt
	var target := Vector2.ZERO if grounded else velocity.limit_length(1200.0) / 1200.0
	travel = travel.lerp(target, 1.0 - exp(-18.0 * dt))

func pose(center: Vector2, grounded: bool) -> Transform2D:
	var motion := travel.length()
	var axis := travel.normalized() if motion > 0.001 else Vector2.UP
	var stretch := motion * TRAVEL_LIMIT
	if kick_strength > 0.0 and kick_age < windup + KICK_RECOVERY:
		axis = kick_axis
		var squeeze := -kick_strength * 0.53
		if kick_age < windup:
			stretch = squeeze * smoothstep(0.0, minf(windup, 0.028), kick_age)
		else:
			var push := kick_age - windup
			if push < KICK_STRETCH_RISE:
				stretch = lerpf(squeeze, stretch + kick_strength, smoothstep(0.0, KICK_STRETCH_RISE, push))
			else:
				var t := (push - KICK_STRETCH_RISE) / (KICK_RECOVERY - KICK_STRETCH_RISE)
				# A short overshoot before the body settles back to travel.
				stretch += kick_strength * cos(t * TAU * 0.83) * pow(1.0 - t, 2.0)
	if land_age < LAND_RECOVERY:
		axis = Vector2.DOWN
		var t := land_age / LAND_RECOVERY
		stretch = -land_strength * cos(t * TAU * 0.88) * pow(1.0 - t, 2.0)
	var along := 1.0 + clampf(stretch, -0.26, 0.38)
	var across := 1.0 / along
	# Project onto the motion axis without rotating the whole resting square.
	var x := Vector2.RIGHT * across + axis * axis.x * (along - across)
	var y := Vector2.DOWN * across + axis * axis.y * (along - across)
	if grounded:
		# Keep the lowest corner on the unchanged collision floor.
		center.y += T.HALF * (1.0 - absf(x.y) - absf(y.y))
	return Transform2D(x, y, center)

static func edge(transform: Transform2D, direction: Vector2) -> Vector2:
	# Invert the scaled basis, without applying translation to the aim vector.
	var local := transform.affine_inverse().basis_xform(direction)
	var distance := T.HALF / maxf(maxf(absf(local.x), absf(local.y)), 0.0001)
	return transform.origin + direction * distance
