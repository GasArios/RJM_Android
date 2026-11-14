#!/usr/bin/env bash
set -euo pipefail
app=com.gasarios.rjm.lab
mkdir -p lab-smoke
trap 'adb logcat -d > lab-smoke/logcat.txt; adb exec-out screencap -p > lab-smoke/final.png' EXIT
adb install -r lab-test/RJM-Godot-Lab-x86_64.apk
adb shell settings put secure immersive_mode_confirmations confirmed
adb shell am force-stop com.google.android.apps.nexuslauncher
adb logcat -c
launcher="$(adb shell cmd package resolve-activity --brief "$app" | tr -d '\r' | tail -1)"
adb shell am start -W -n "$launcher"

wait_log() {
  for i in $(seq 1 24); do
    adb logcat -d > lab-smoke/current.txt
    if grep -q "$1" lab-smoke/current.txt; then return 0; fi
    sleep 1
  done
  echo "Missing Android log: $1" >&2
  return 1
}
no_shots() {
  adb logcat -d > lab-smoke/current.txt
  if grep -q 'RJM_GODOT shot' lab-smoke/current.txt; then
    echo "Unexpected pad shot" >&2
    return 1
  fi
}

wait_log 'RJM_GODOT ready'
sleep 1
adb exec-out screencap -p > lab-smoke/range.png
python3 - <<'PY' > lab-smoke/coords.sh
import struct
from pathlib import Path
w,h=struct.unpack('>II',Path('lab-smoke/range.png').read_bytes()[16:24])
assert w>h, (w,h)
s=min(w/1280,h/720)
for k,x,y in [('center',w-150*s,h-156*s),('down',w-150*s,h-99*s),('world',w*.48,h*.4),('guns',97*s,h-103*s),('reload',97*s,h-177*s),('shotgun',w*.5+147*s,h*.5-1*s),('knob',w*.5,h*.5+139*s),('resume',w*.5,h*.5-62*s)]:
 print(f'{k}="{round(x)} {round(y)}"')
PY
source lab-smoke/coords.sh
# If Android briefly removed focus during startup, the app correctly paused.
if grep -q 'RJM_GODOT menu=pause' lab-smoke/current.txt; then adb shell input tap $resume; fi
adb shell input tap $world
adb shell input tap $center
sleep 1
no_shots
adb shell input tap $down
wait_log 'RJM_GODOT shot REVOLVER'
adb shell input tap $guns
wait_log 'RJM_GODOT menu=guns'
adb exec-out screencap -p > lab-smoke/guns.png
adb shell input tap $shotgun
wait_log 'RJM_GODOT selected SHOTGUN'
adb shell input tap $down
wait_log 'RJM_GODOT shot SHOTGUN'
adb shell input tap $reload
wait_log 'RJM_GODOT reload queued'
wait_log 'RJM_GODOT reloaded'
adb shell input keyevent KEYCODE_BACK
wait_log 'RJM_GODOT menu=pause'
adb exec-out screencap -p > lab-smoke/pause.png
adb shell input tap $knob
wait_log 'RJM_GODOT knob=0'
adb shell am force-stop "$app"
adb logcat -c
adb shell am start -W -n "$launcher"
wait_log 'RJM_GODOT ready.*knob=0'
adb shell input keyevent KEYCODE_HOME
wait_log 'RJM_GODOT menu=pause'
adb shell am start -W -n "$launcher"
adb shell pidof "$app"
! grep -E 'FATAL EXCEPTION|Fatal signal|SCRIPT ERROR' lab-smoke/current.txt
echo "PASS Android Godot install, touch, guns, reload, Back, focus and setting restore"
