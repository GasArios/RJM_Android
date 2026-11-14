"""Restore the one persistent signing identity from GitHub Actions secrets.

No secret values or key bytes are logged. Missing secrets allow validation only;
a partially configured or wrong signing identity fails instead of changing keys.
"""
import base64
import binascii
import hashlib
import os
from pathlib import Path
import subprocess
import sys


def prepare(env, root, pin):
    encoded = env.get('RJM_ANDROID_KEYSTORE_BASE64', '')
    password = env.get('RJM_ANDROID_KEYSTORE_PASSWORD', '')
    if not encoded and not password:
        return None
    if not encoded or not password:
        raise ValueError('Register both RJM_ANDROID_KEYSTORE_BASE64 and RJM_ANDROID_KEYSTORE_PASSWORD.')
    try:
        data = base64.b64decode(encoded, validate=True)
    except (ValueError, binascii.Error):
        raise ValueError('The keystore secret must contain valid Base64.') from None
    if not 128 <= len(data) <= 35000:
        raise ValueError('The keystore secret has an invalid size.')
    root.mkdir(mode=0o700, parents=True, exist_ok=True)
    path = root / 'rjm-signing.p12'
    fd = os.open(path, os.O_WRONLY | os.O_CREAT | os.O_TRUNC, 0o600)
    with os.fdopen(fd, 'wb') as file:
        file.write(data)
    result = subprocess.run(['keytool', '-exportcert', '-keystore', str(path),
        '-storepass:env', 'RJM_ANDROID_KEYSTORE_PASSWORD', '-alias', 'rjm'],
        env=dict(env), capture_output=True)
    if result.returncode:
        path.unlink(missing_ok=True)
        raise ValueError('Cannot open the signing key with the configured password and rjm alias.')
    actual = hashlib.sha256(result.stdout).hexdigest()
    if actual != pin.read_text().strip().lower():
        path.unlink(missing_ok=True)
        raise ValueError('Signing certificate differs from the pinned RJM identity. Do not replace the key.')
    return path


def main():
    try:
        path = prepare(os.environ, Path(os.environ['RUNNER_TEMP']) / 'rjm-private-signing',
                       Path('config/android-signing-certificate.sha256'))
        persistent = path is not None
        if not persistent:
            # Exercise the same custom Gradle signing path in validation-only CI.
            # This temporary fixture is never an authorized release identity.
            directory = Path(os.environ['RUNNER_TEMP']) / 'rjm-private-signing'
            directory.mkdir(mode=0o700, parents=True, exist_ok=True)
            path = directory / 'rjm-signing.p12'
            validation_env = dict(os.environ, RJM_ANDROID_KEYSTORE_PASSWORD='rjm-ci-fixture-only')
            result = subprocess.run(['keytool', '-genkeypair', '-alias', 'rjm', '-keyalg', 'RSA', '-keysize', '2048',
                '-validity', '30', '-dname', 'CN=RJM temporary CI validation', '-storetype', 'PKCS12',
                '-keystore', str(path), '-storepass:env', 'RJM_ANDROID_KEYSTORE_PASSWORD'],
                env=validation_env, capture_output=True)
            if result.returncode:
                raise ValueError('Cannot create temporary validation signing fixture.')
            path.chmod(0o600)
        with open(os.environ['GITHUB_OUTPUT'], 'a') as output:
            output.write('persistent_signing=' + ('true' if persistent else 'false') + '\n')
        with open(os.environ['GITHUB_ENV'], 'a') as output:
            output.write('RJM_ANDROID_KEYSTORE_PATH=' + str(path) + '\n')
        if persistent:
            print('Persistent signing certificate verified. Private key stays in runner temporary storage.')
        else:
            print('::notice::Signing secrets are not configured. This run validates only; no APK release will be published.')
            with open(os.environ['GITHUB_STEP_SUMMARY'], 'a') as output:
                output.write('## One-time signing setup needed\nRegister the two repository Actions secrets described in docs/android-updates.md. This run is validation only and cannot publish a random-key APK.\n')
    except (KeyError, OSError, ValueError):
        # Never print arbitrary environment/file content in exception output.
        print('::error::Persistent signing setup is incomplete or does not match the pinned certificate. See docs/android-updates.md.', file=sys.stderr)
        return 1
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
