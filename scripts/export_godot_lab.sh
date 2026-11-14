#!/usr/bin/env bash
set -euo pipefail
preset="godot-lab/export_presets.cfg"
tools="$ANDROID_HOME/build-tools/35.0.1"
unsigned="$RUNNER_TEMP/rjm-godot-unsigned"
mkdir -p "$unsigned" lab-dist lab-test
sed -i "s/^version\/code=.*/version\/code=$GITHUB_RUN_NUMBER/" "$preset"
"$RJM_GODOT" --headless --path godot-lab --export-release Android "$unsigned/arm64.apk"
test -s "$unsigned/arm64.apk"
sed -i 's/^architectures\/arm64-v8a=true$/architectures\/arm64-v8a=false/;s/^architectures\/x86_64=false$/architectures\/x86_64=true/' "$preset"
"$RJM_GODOT" --headless --path godot-lab --export-release Android "$unsigned/x86_64.apk"
test -s "$unsigned/x86_64.apk"

# Keep signing outside Godot export so the project's presets never contain secrets.
for abi in arm64 x86_64; do
  target="lab-dist/RJM-Godot-Lab.apk"
  if [[ "$abi" == x86_64 ]]; then target="lab-test/RJM-Godot-Lab-x86_64.apk"; fi
  "$tools/zipalign" -P 16 -f 4 "$unsigned/$abi.apk" "$unsigned/aligned.apk"
  "$tools/apksigner" sign --v4-signing-enabled false --ks "$RJM_ANDROID_KEYSTORE_PATH" --ks-key-alias rjm \
    --ks-pass env:RJM_ANDROID_KEYSTORE_PASSWORD --key-pass env:RJM_ANDROID_KEYSTORE_PASSWORD \
    --out "$target" "$unsigned/aligned.apk"
  "$tools/apksigner" verify --verbose --print-certs "$target" > "$target.certs"
  digest="$(sed -n 's/^Signer #1 certificate SHA-256 digest: //p' "$target.certs")"
  test "$digest" = "$(cat config/android-signing-certificate.sha256)"
  "$tools/aapt" dump badging "$target" > "$target.badging"
  grep -q "package: name='com.gasarios.rjm.lab' versionCode='$GITHUB_RUN_NUMBER'" "$target.badging"
  expected="$abi"
  if [[ "$abi" == arm64 ]]; then expected="arm64-v8a"; fi
  grep -q "^native-code: '$expected'" "$target.badging"
  "$tools/zipalign" -c -P 16 4 "$target"
  rm "$target.certs" "$target.badging"
done
(cd lab-dist && sha256sum RJM-Godot-Lab.apk > SHA256SUMS.txt)
