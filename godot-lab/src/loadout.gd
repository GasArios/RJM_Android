extends RefCounted
## One magazine and cooldown per weapon. A queued reload only runs grounded.
const T = preload("res://src/tuning.gd")

var selected := 0
var ammo: Array[int] = [6, 2]
var cooldown: Array[float] = [0.0, 0.0]
var queued := false
var reload_left := 0.0
var shot_count := 0
var equipped_count := 2

func lock_shotgun() -> void:
	equipped_count = 1
	selected = 0
	ammo[1] = 0

func unlock_shotgun() -> void:
	if equipped_count == 2:
		return
	equipped_count = 2
	ammo[1] = int(T.GUNS[1].ammo)
	cooldown[1] = 0.0

func select(slot: int) -> bool:
	if slot < 0 or slot >= equipped_count:
		return false
	selected = slot
	return true

func reload_seconds() -> float:
	var total := 0.0
	for slot in range(equipped_count):
		total += float(T.GUNS[slot].reload)
	return clampf(total / equipped_count, 0.20, 2.0)

func request_reload() -> void:
	if reload_left > 0.0:
		return
	for slot in range(equipped_count):
		if ammo[slot] < int(T.GUNS[slot].ammo):
			queued = true
			return

func try_fire() -> int:
	if reload_left > 0.0 or queued:
		return -1
	if ammo[selected] == 0:
		if equipped_count == 1:
			request_reload()
			return -1
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
		reload_left = reload_seconds()
		return false
	if reload_left > 0.0:
		reload_left = maxf(reload_left - dt, 0.0)
		if reload_left <= 0.00001:
			reload_left = 0.0
			for i in range(equipped_count):
				ammo[i] = int(T.GUNS[i].ammo)
			return true
	return false
