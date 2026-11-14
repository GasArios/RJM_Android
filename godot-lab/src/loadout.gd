extends RefCounted
## One magazine and cooldown per weapon. A queued reload only runs grounded.
const T = preload("res://src/tuning.gd")

var selected := 0
var ammo: Array[int] = [6, 2]
var cooldown: Array[float] = [0.0, 0.0]
var queued := false
var reload_left := 0.0
var shot_count := 0

func request_reload() -> void:
	if reload_left <= 0.0 and (ammo[0] < int(T.GUNS[0].ammo) or ammo[1] < int(T.GUNS[1].ammo)):
		queued = true

func try_fire() -> int:
	if reload_left > 0.0 or queued:
		return -1
	if ammo[selected] == 0:
		var other := 1 - selected
		if ammo[other] > 0 and cooldown[other] <= 0.00001:
			selected = other
		elif ammo[other] == 0:
			request_reload()
			return -1
		else:
			return -1
	if cooldown[selected] > 0.00001:
		return -1
	ammo[selected] -= 1
	cooldown[selected] = float(T.GUNS[selected].cooldown)
	shot_count += 1
	return selected

func advance(dt: float, grounded: bool) -> bool:
	for i in range(cooldown.size()):
		cooldown[i] = maxf(cooldown[i] - dt, 0.0)
	if not grounded:
		return false
	if queued:
		queued = false
		reload_left = clampf((float(T.GUNS[0].reload)+float(T.GUNS[1].reload))*0.5, 0.20, 2.0)
		return false
	if reload_left > 0.0:
		reload_left = maxf(reload_left - dt, 0.0)
		if reload_left <= 0.00001:
			reload_left = 0.0
			for i in range(ammo.size()):
				ammo[i] = int(T.GUNS[i].ammo)
			return true
	return false
