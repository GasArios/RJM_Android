#!/usr/bin/env bash
set -euo pipefail
mkdir -p smoke-output
adb install -r upgrade-dist/RJM-Android-Upgrade-Baseline.apk
adb shell run-as com.gasarios.rjm mkdir -p files
# Seed a valid checkpoint in the older install. The gameplay smoke test
# separately verifies that the upgraded app can read and write checkpoints.
printf 'RJM_CHECKPOINT_2\n48 208 2 1 1 0 1.1\n' | adb shell "run-as com.gasarios.rjm sh -c 'cat > files/checkpoint-v1.txt'"
adb exec-out run-as com.gasarios.rjm cat files/checkpoint-v1.txt > smoke-output/upgrade-checkpoint-before.txt
adb shell "run-as com.gasarios.rjm sh -c 'echo rjm-update-data-preserved > files/update-sentinel.txt'"
adb install -r dist/RJM-Android-0.1.2.apk
adb exec-out run-as com.gasarios.rjm cat files/checkpoint-v1.txt > smoke-output/upgrade-checkpoint-after.txt
cmp smoke-output/upgrade-checkpoint-before.txt smoke-output/upgrade-checkpoint-after.txt
adb exec-out run-as com.gasarios.rjm cat files/update-sentinel.txt > smoke-output/upgrade-sentinel.txt
grep -qx 'rjm-update-data-preserved' smoke-output/upgrade-sentinel.txt
adb shell dumpsys package com.gasarios.rjm > smoke-output/upgrade-package.txt
python3 - <<'PY'
from pathlib import Path
import json,re
expected=json.loads(Path('dist/upgrade-validation.json').read_text())
actual=re.search(r'versionCode=(\d+)',Path('smoke-output/upgrade-package.txt').read_text())
assert actual and int(actual[1])==expected['after']['version_code']
Path('smoke-output/upgrade-result.txt').write_text('PASS APK overwrite upgrade retains checkpoint and private app data; versionCode '+str(expected['before']['version_code'])+' -> '+str(expected['after']['version_code'])+'\n')
print(Path('smoke-output/upgrade-result.txt').read_text())
PY
