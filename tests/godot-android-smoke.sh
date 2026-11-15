#!/usr/bin/env bash
set -euo pipefail
app=com.gasarios.rjm.lab
mkdir -p lab-smoke
trap 'adb logcat -d > lab-smoke/logcat.txt; adb exec-out screencap -p > lab-smoke/final.png' EXIT
adb install -r lab-test/RJM-Godot-Lab-x86_64.apk
adb shell settings put system accelerometer_rotation 0
adb shell settings put system user_rotation 1
adb shell settings put secure immersive_mode_confirmations confirmed
adb shell am force-stop com.google.android.apps.nexuslauncher
launcher="$(adb shell cmd package resolve-activity --brief "$app" | tr -d '\r' | tail -1)"

wait_log() {
  for i in $(seq 1 24); do
    adb logcat -d > lab-smoke/current.txt
    if grep -q "$1" lab-smoke/current.txt; then return 0; fi
    sleep 1
  done
  echo "Missing Android log: $1" >&2
  return 1
}

launch_fresh() {
  # On a cold emulator, Android can recreate the activity during the first
  # landscape transition. Wait for the Godot scene itself to report ready.
  for _ in $(seq 1 2); do
    adb shell am force-stop "$app"
    adb logcat -c
    adb shell am start -W -n "$launcher"
    if wait_log 'RJM_GODOT ready'; then return 0; fi
  done
  return 1
}
no_shots() {
  adb logcat -d > lab-smoke/current.txt
  if grep -q 'RJM_GODOT shot' lab-smoke/current.txt; then
    echo "Unexpected pad shot" >&2
    return 1
  fi
}

capture() {
  local name=$1
  for _ in $(seq 1 8); do
    adb exec-out screencap -p > "lab-smoke/$name.png"
    if python3 - "lab-smoke/$name.png" <<'PY'
from pathlib import Path
import struct
import sys
import zlib

# Decode the screenshot enough to distinguish an actual game frame from a
# uniform clear/splash frame, without depending on Ubuntu image packages.
png = Path(sys.argv[1]).read_bytes()
assert png[:8] == b'\x89PNG\r\n\x1a\n', 'invalid screenshot'
width, height, depth, color = struct.unpack_from('>IIBB', png, 16)
assert width > height and depth == 8 and color in (2, 6), 'expected landscape RGB screenshot'
channels = 4 if color == 6 else 3
cursor = 8
data = bytearray()
while cursor + 12 <= len(png):
    n, kind = struct.unpack_from('>I4s', png, cursor)
    if kind == b'IDAT':
        data.extend(png[cursor+8:cursor+8+n])
    cursor += n + 12
pixels = zlib.decompress(data)
assert len(pixels) == (width * channels + 1) * height, 'unexpected PNG layout'
# Filtered scanline bytes of a flat frame are almost entirely zero (or show a
# single constant pixel value repeated). Detailed frames vary across many rows.
stride = width * channels + 1
varied_rows = sum(len(set(pixels[i+1:i+stride])) >= 24 for i in range(0, len(pixels), stride))
assert varied_rows >= max(12, height // 20), f'blank or unrendered frame: {varied_rows} varied rows'
PY
    then return 0; fi
    sleep 1
  done
  echo "Failed to capture a rendered frame: $name" >&2
  return 1
}

sleep 3
launch_fresh
sleep 2
capture range
python3 - <<'PY' > lab-smoke/coords.sh
import struct
from pathlib import Path
w,h=struct.unpack('>II',Path('lab-smoke/range.png').read_bytes()[16:24])
assert w>h, (w,h)
s=min(w/1280,h/720)
for k,x,y in [('center',w-150*s,h-156*s),('down',w-150*s,h-99*s),('world',w*.48,h*.4),('guns',97*s,h-103*s),('reload',97*s,h-177*s),('shotgun',w*.5+147*s,h*.5-1*s),('knob',w*.5,h*.5+139*s),('resume',w*.5,h*.5-62*s),('mode',w*.5-150*s,h*.5+202*s),('shotgun_card',w*.5+145*s,h*.5),('retry',w*.5,h*.5+5*s)]:
 print(f'{k}="{round(x)} {round(y)}"')
for k,x,y in [('jump_low',-31,57),('travel',-65,0),('approach_high',-61,-22),('jump_high',-20,62)]:
 print(f'{k}="{round(w+(x-150)*s)} {round(h+(y-156)*s)}"')
PY
source lab-smoke/coords.sh
# If Android briefly removed focus during startup, the app correctly paused.
if grep -q 'RJM_GODOT menu=pause' lab-smoke/current.txt; then adb shell input tap $resume; fi
wait_log 'RJM_GODOT harbor stage=0'
adb shell input tap $guns
wait_log 'RJM_GODOT menu=guns'
capture locked-shotgun
adb shell input tap $shotgun_card
sleep 0.4
adb logcat -d > lab-smoke/current.txt
if grep -q 'RJM_GODOT selected SHOTGUN' lab-smoke/current.txt; then
  echo "Locked shotgun selected before the supply case" >&2
  exit 1
fi
adb shell input keyevent KEYCODE_BACK
adb shell input tap $down
wait_log 'RJM_GODOT harbor stage=1'
capture route
launch_fresh
wait_log 'RJM_GODOT harbor stage=1'
if grep -q 'RJM_GODOT menu=pause' lab-smoke/current.txt; then adb shell input tap $resume; fi
adb shell input tap $jump_low
wait_log 'RJM_GODOT harbor stage=2'
sleep 1.6
adb shell input tap $travel
wait_log 'RJM_GODOT harbor stage=3'
sleep 1.6
capture supply
adb shell input tap $approach_high
wait_log 'RJM_GODOT shot SHOTGUN'
sleep 2.0
adb shell input tap $jump_high
wait_log 'RJM_GODOT harbor stage=4'
capture high-pier
launch_fresh
wait_log 'RJM_GODOT harbor stage=4'
# Start the existing control regression in the untouched free-practice range.
if ! grep -q 'RJM_GODOT menu=pause' lab-smoke/current.txt; then adb shell input keyevent KEYCODE_BACK; fi
wait_log 'RJM_GODOT menu=pause'
adb shell input tap $retry
wait_log 'RJM_GODOT menu=world'
adb shell input keyevent KEYCODE_BACK
sleep 0.4
adb shell input tap $mode
wait_log 'RJM_GODOT free practice'
adb logcat -c
adb shell input tap $world
adb shell input tap $center
sleep 1
no_shots
adb shell input tap $down
wait_log 'RJM_GODOT shot REVOLVER'
sleep 0.2
capture recoil
adb shell input tap $guns
wait_log 'RJM_GODOT menu=guns'
capture guns
adb shell input tap $shotgun
wait_log 'RJM_GODOT selected SHOTGUN'
adb shell input tap $down
wait_log 'RJM_GODOT shot SHOTGUN'
adb shell input tap $reload
wait_log 'RJM_GODOT reload queued'
wait_log 'RJM_GODOT reloaded'
adb shell input keyevent KEYCODE_BACK
wait_log 'RJM_GODOT menu=pause'
capture pause
adb shell input tap $knob
wait_log 'RJM_GODOT knob=0'
launch_fresh
wait_log 'RJM_GODOT ready.*knob=0'
adb shell input keyevent KEYCODE_HOME
wait_log 'RJM_GODOT menu=pause'
adb shell am start -W -n "$launcher"
adb shell pidof "$app"
! grep -E 'FATAL EXCEPTION|Fatal signal|SCRIPT ERROR|CanvasShaderGLES3: Program linking failed' lab-smoke/current.txt
echo "PASS Android Godot rendered frames, pier travel, shotgun unlock, high-pier checkpoint and practice controls"
