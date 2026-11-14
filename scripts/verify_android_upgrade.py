"""Check APK identity and monotonic versions before the emulator upgrade test."""
import json
import os
from pathlib import Path
import re
import subprocess
import sys


def inspect(apk, tools):
    meta = subprocess.check_output([str(tools / 'aapt'), 'dump', 'badging', str(apk)], text=True)
    package = re.search(r"package: name='([^']+)' versionCode='(\d+)'", meta)
    cert = subprocess.check_output([str(tools / 'apksigner'), 'verify', '--print-certs', str(apk)], text=True)
    fingerprint = re.search(r'Signer #1 certificate SHA-256 digest: ([a-fA-F0-9]+)', cert)
    if not package or not fingerprint:
        raise ValueError('APK metadata or certificate missing')
    return {'package': package[1], 'version_code': int(package[2]), 'certificate_sha256': fingerprint[1].lower()}


def verify(before, after, pin=None):
    if before['package'] != after['package'] or after['package'] != 'com.gasarios.rjm':
        raise ValueError('APK applicationId changed')
    if before['certificate_sha256'] != after['certificate_sha256']:
        raise ValueError('APK signing certificate changed')
    if after['version_code'] <= before['version_code']:
        raise ValueError('APK versionCode must increase')
    if pin is not None and after['certificate_sha256'] != pin:
        raise ValueError('APK does not use the pinned persistent signing key')


def main():
    tools = Path(os.environ['ANDROID_HOME']) / 'build-tools/35.0.0'
    before = inspect(Path('upgrade-dist/RJM-Android-Upgrade-Baseline.apk'), tools)
    after = inspect(Path('dist/RJM-Android-0.1.2.apk'), tools)
    persistent = sys.argv[1] == 'true'
    pin = Path('config/android-signing-certificate.sha256').read_text().strip() if persistent else None
    verify(before, after, pin)
    Path('dist/upgrade-validation.json').write_text(json.dumps({'persistent_signing': persistent, 'before': before, 'after': after}, indent=2) + '\n')
    print('PASS same package/certificate and increasing versionCode: ' + str(before['version_code']) + ' -> ' + str(after['version_code']))


if __name__ == '__main__':
    main()
