#!/usr/bin/env bash
set -euo pipefail
set -x
mkdir -p smoke-output
trap 'adb logcat -d > smoke-output/logcat.txt; adb exec-out screencap -p > smoke-output/final.png' EXIT
adb install -r dist/RJM-Android-0.1.2.apk
adb shell settings put system accelerometer_rotation 0
adb shell settings put system user_rotation 1
adb shell settings put secure immersive_mode_confirmations confirmed
# The cold emulator's Google launcher can show its own ANR during first boot.
# Start the game after initial system setup rather than interacting with that dialog.
adb shell am force-stop com.google.android.apps.nexuslauncher
sleep 10
adb logcat -c
adb shell am start -W -n com.gasarios.rjm/.RjmActivity
sleep 5
adb logcat -d > smoke-output/start.txt
grep -q 'RJM: ready' smoke-output/start.txt
! grep -E 'Fatal signal|FATAL EXCEPTION' smoke-output/start.txt
adb exec-out screencap -p > smoke-output/gameplay.png
# Derive the full-width viewport and centered UI from the screenshot.
python3 - <<'PY' > smoke-output/points.sh
import struct
data=open('smoke-output/gameplay.png','rb').read(); w,h=struct.unpack('>II',data[16:24])
assert w>h, f'Expected landscape, got {w}x{h}'
s=h/720; shift=(w/s-1280)/2; ox=oy=0
for name,x,y in [('FIRE',640,680),('GUNS',70,590),('SHOTGUN',620+shift,360),('RESUME',640+shift,235)]:
 print(f'{name}_X={round(ox+x*s)}; {name}_Y={round(oy+y*s)}')
PY
source smoke-output/points.sh
if adb logcat -d | grep 'RJM: menu=2' > /dev/null; then
  adb shell input tap "$RESUME_X" "$RESUME_Y"
  sleep 1
fi
adb shell input tap "$FIRE_X" "$FIRE_Y"
sleep 1
adb logcat -d | grep -q 'RJM: fire'
adb shell input tap "$GUNS_X" "$GUNS_Y"
sleep 1
adb exec-out screencap -p > smoke-output/weapons.png
adb logcat -d | grep -q 'RJM: menu=1'
adb shell input tap "$SHOTGUN_X" "$SHOTGUN_Y"
sleep 1
adb shell input keyevent KEYCODE_BACK
sleep 1
adb exec-out screencap -p > smoke-output/pause.png
adb logcat -d | grep -q 'RJM: menu=2'
adb shell input tap "$RESUME_X" "$RESUME_Y"
sleep 1
adb logcat -c
adb shell input keyevent KEYCODE_HOME
sleep 1
adb shell am force-stop com.google.android.apps.nexuslauncher
adb shell am start -W -n com.gasarios.rjm/.RjmActivity
sleep 2
adb exec-out screencap -p > smoke-output/resumed-paused.png
adb logcat -d > smoke-output/end.txt
grep -q 'RJM: menu=2' smoke-output/end.txt
! grep -q 'RJM: fire' smoke-output/end.txt
! grep -E 'Fatal signal|FATAL EXCEPTION' smoke-output/end.txt
adb shell pidof com.gasarios.rjm
echo 'PASS APK install, native launch, touch shot, weapon UI, Back and app resume'

adb install -r test-dist/app-debug-androidTest.apk
adb logcat -c
adb shell am instrument -w com.gasarios.rjm.test/com.gasarios.rjm.ControlSmoke | tee smoke-output/instrumentation.txt
grep -q 'PASS native two-finger pinch' smoke-output/instrumentation.txt
adb exec-out run-as com.gasarios.rjm cat files/control-smoke.png > smoke-output/pinch-settings.png
for name in pad-3-held pad-4-held pad-3-released pad-4-released pad-reload-blocked four-mode-settings; do
  adb exec-out run-as com.gasarios.rjm cat "files/$name.png" > "smoke-output/$name.png"
done
adb exec-out run-as com.gasarios.rjm cat files/checkpoint-v1.txt > smoke-output/checkpoint.txt
adb shell am force-stop com.gasarios.rjm
adb logcat -c
adb shell am start -W -n com.gasarios.rjm/.RjmActivity
sleep 3
adb logcat -d > smoke-output/restored.txt
python3 - <<'PY'
import re
fields=open('smoke-output/checkpoint.txt').read().split()
logs=open('smoke-output/restored.txt').read()
match=re.search(r'RJM: restored aim=(\d+) zoom=([\d.]+)',logs)
assert match and int(match[1])==int(fields[6]) and abs(float(match[2])-float(fields[7]))<0.002, 'Aim/zoom not restored'
assert 'Fatal signal' not in logs and 'FATAL EXCEPTION' not in logs
print('PASS process restart restores saved aim mode and zoom')
PY
