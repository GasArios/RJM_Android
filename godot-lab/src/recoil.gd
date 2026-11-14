extends RefCounted
## Screen/world Y increases downward; the raylib formulas use Y-up.
const T = preload("res://src/tuning.gd")

static func impulse(velocity: Vector2, direction: Vector2, force: float,
		grounded: bool, brace: bool, climb: float, resist: float) -> Vector2:
	var kick := -direction.normalized() * force
	if kick.y < 0.0 and not grounded:
		kick.y *= altitude_factor(climb / (1.0 + resist * 0.75))
	if grounded and brace:
		kick.x *= T.BRACE_X
		kick.y = minf(kick.y, 0.0) * T.BRACE_UP
		if absf(kick.y) <= T.BRACE_ANCHOR:
			kick.y = 0.0
	velocity.x = clampf(velocity.x + kick.x, -T.MAX_X, T.MAX_X)
	if kick.y < 0.0:
		velocity.y = minf(velocity.y, kick.y) + minf(0.0, maxf(velocity.y, kick.y)) * T.SYNERGY
	elif kick.y > 0.0:
		velocity.y = maxf(velocity.y, kick.y) + maxf(0.0, minf(velocity.y, kick.y)) * T.SYNERGY
	velocity.y = clampf(velocity.y, -T.MAX_UP, T.MAX_STOMP)
	return velocity

static func altitude_factor(height: float) -> float:
	var xs := [400.0, 500.0, 650.0, 800.0, 900.0]
	var ys := [1.0, 0.73, 0.53, 0.38, 0.30]
	if height <= xs[0]:
		return 1.0
	for i in range(1, xs.size()):
		if height <= xs[i]:
			return lerpf(ys[i-1], ys[i], inverse_lerp(xs[i-1], xs[i], height))
	return 0.30

static func forces(velocity: Vector2, dt: float, grounded: bool, brace: bool) -> Vector2:
	if velocity.y < -T.APEX:
		velocity.y = velocity.y * pow(T.RISE_DRAG, dt) + T.GRAVITY * dt
	elif velocity.y <= T.APEX:
		velocity.y += T.GRAVITY * T.APEX_GRAVITY * dt
	else:
		velocity.y += T.GRAVITY * dt
	velocity.y = clampf(velocity.y, -T.MAX_UP, T.MAX_FALL)
	velocity.x *= pow(T.GROUND_DRAG if grounded else T.AIR_DRAG, dt)
	if absf(velocity.x) < 3.0:
		velocity.x = 0.0
	if grounded and brace:
		velocity.x *= pow(T.BRACE_DRAG, dt)
		if absf(velocity.x) < 14.0:
			velocity.x = 0.0
	return velocity
