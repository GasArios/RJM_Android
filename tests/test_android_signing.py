import base64
import hashlib
import importlib.util
import os
from pathlib import Path
import subprocess
import tempfile
import unittest


def module(name):
    spec = importlib.util.spec_from_file_location(name, Path(__file__).parents[1] / 'scripts' / (name + '.py'))
    result = importlib.util.module_from_spec(spec); spec.loader.exec_module(result)
    return result


signing = module('prepare_android_signing')
upgrade = module('verify_android_upgrade')


class SigningTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.root = Path(cls.temp.name)
        cls.env = dict(os.environ, RJM_ANDROID_KEYSTORE_PASSWORD='temporary-test-fixture-only')
        cls.key = cls.root / 'fixture.p12'
        subprocess.run(['keytool', '-genkeypair', '-alias', 'rjm', '-keyalg', 'RSA', '-keysize', '2048', '-validity', '30',
            '-dname', 'CN=Temporary signing tests', '-storetype', 'PKCS12', '-keystore', str(cls.key),
            '-storepass:env', 'RJM_ANDROID_KEYSTORE_PASSWORD'], env=cls.env, check=True, capture_output=True)
        cert = subprocess.check_output(['keytool', '-exportcert', '-keystore', str(cls.key),
            '-storepass:env', 'RJM_ANDROID_KEYSTORE_PASSWORD', '-alias', 'rjm'], env=cls.env)
        cls.pin = cls.root / 'fingerprint'; cls.pin.write_text(hashlib.sha256(cert).hexdigest())
        cls.env['RJM_ANDROID_KEYSTORE_BASE64'] = base64.b64encode(cls.key.read_bytes()).decode()

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def test_absent_secrets_are_validation_only(self):
        self.assertIsNone(signing.prepare({}, self.root / 'absent', self.pin))
        self.assertFalse((self.root / 'absent').exists())

    def test_partial_or_invalid_secrets_fail(self):
        for env in [{'RJM_ANDROID_KEYSTORE_PASSWORD': 'x'}, {'RJM_ANDROID_KEYSTORE_BASE64': 'x'},
                    {'RJM_ANDROID_KEYSTORE_PASSWORD': 'x', 'RJM_ANDROID_KEYSTORE_BASE64': '!!!'}]:
            with self.assertRaises(ValueError): signing.prepare(env, self.root / 'invalid', self.pin)

    def test_valid_identity_is_restored_privately(self):
        target = signing.prepare(self.env, self.root / 'valid', self.pin)
        self.assertEqual(target.read_bytes(), self.key.read_bytes())
        self.assertEqual(target.stat().st_mode & 0o777, 0o600)

    def test_wrong_password_or_changed_key_fails(self):
        for env, pin in [(dict(self.env, RJM_ANDROID_KEYSTORE_PASSWORD='wrong'), self.pin),
                         (self.env, self.root / 'wrong-pin')]:
            pin.write_text('0' * 64) if pin.name == 'wrong-pin' else None
            target = self.root / 'rejected'
            with self.assertRaises(ValueError): signing.prepare(env, target, pin)
            self.assertFalse((target / 'rjm-signing.p12').exists())

    def test_upgrade_identity_and_version(self):
        before = {'package': 'com.gasarios.rjm', 'version_code': 10, 'certificate_sha256': 'abc'}
        after = dict(before, version_code=11)
        upgrade.verify(before, after, 'abc')
        for bad in [dict(after, package='different'), dict(after, certificate_sha256='def'), dict(after, version_code=10)]:
            with self.assertRaises(ValueError): upgrade.verify(before, bad)
        with self.assertRaises(ValueError): upgrade.verify(before, after, 'wrong')


if __name__ == '__main__':
    unittest.main(verbosity=2)
