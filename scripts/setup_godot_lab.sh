#!/usr/bin/env bash
set -euo pipefail
dir="$RUNNER_TEMP/rjm-godot"
tag="4.6.3-stable"
editor="Godot_v${tag}_linux.x86_64"
templates="Godot_v${tag}_export_templates.tpz"
url="https://github.com/godotengine/godot-builds/releases/download/$tag"
mkdir -p "$dir"
curl -fLsS --retry 3 "$url/$editor.zip" -o "$dir/editor.zip"
curl -fLsS --retry 3 "$url/$templates" -o "$dir/templates.tpz"
(
  cd "$dir"
  printf '%s\n' \
    'd0bc2113065e481c9c2c2b2c37daa4e8be3fe9e27f0ab9ab0b6096e9a37907f3  editor.zip' \
    '3fbe2c0e2dec9d537ab9ec97bcf8da91dcf23357fc51f67092dd068d839290a8  templates.tpz' | sha256sum -c
  unzip -q editor.zip
)
chmod +x "$dir/$editor"
template_dir="$HOME/.local/share/godot/export_templates/4.6.3.stable"
mkdir -p "$template_dir" "$HOME/.config/godot"
unzip -jqo "$dir/templates.tpz" 'templates/android_release.apk' 'templates/version.txt' -d "$template_dir"
cat > "$HOME/.config/godot/editor_settings-4.6.tres" <<SETTINGS
[gd_resource type="EditorSettings" format=3]

[resource]
export/android/android_sdk_path = "$ANDROID_HOME"
export/android/java_sdk_path = "$JAVA_HOME"
SETTINGS
printf 'RJM_GODOT=%s\n' "$dir/$editor" >> "$GITHUB_ENV"
"$dir/$editor" --version
