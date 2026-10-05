#!/usr/bin/env bash
set -euo pipefail
mkdir -p smoke-output
trap 'adb logcat -d > smoke-output/logcat.txt; adb exec-out screencap -p > smoke-output/final.png' EXIT
adb install -r dist/RJM-Android-0.1.0.apk
adb shell settings put system accelerometer_rotation 0
adb shell settings put system user_rotation 1
adb logcat -c
adb shell am start -W -n com.gasarios.rjm/.RjmActivity
sleep 5
adb logcat -d > smoke-output/start.txt
grep -q 'RJM: ready' smoke-output/start.txt
! grep -E 'Fatal signal|FATAL EXCEPTION' smoke-output/start.txt
adb exec-out screencap -p > smoke-output/gameplay.png
# Derive letterboxed coordinates from the actual landscape screenshot.
python3 - <<'PY' > smoke-output/points.sh
import struct
data=open('smoke-output/gameplay.png','rb').read(); w,h=struct.unpack('>II',data[16:24])
assert w>h, f'Expected landscape, got {w}x{h}'
s=min(w/1280,h/720); ox=(w-1280*s)/2; oy=(h-720*s)/2
for name,x,y in [('FIRE',640,680),('GUNS',70,590),('SHOTGUN',620,360),('RESUME',640,256)]:
 print(f'{name}_X={round(ox+x*s)}; {name}_Y={round(oy+y*s)}')
PY
source smoke-output/points.sh
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
adb shell am start -W -n com.gasarios.rjm/.RjmActivity
sleep 2
adb exec-out screencap -p > smoke-output/resumed-paused.png
adb logcat -d > smoke-output/end.txt
grep -q 'RJM: menu=2' smoke-output/end.txt
! grep -q 'RJM: fire' smoke-output/end.txt
! grep -E 'Fatal signal|FATAL EXCEPTION' smoke-output/end.txt
adb shell pidof com.gasarios.rjm
echo 'PASS APK install, native launch, touch shot, weapon UI, Back and app resume'
