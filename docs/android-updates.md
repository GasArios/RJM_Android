# APK를 삭제 없이 업데이트하기

Android는 같은 applicationId와 서명 키를 가진 APK를 기존 앱의 업데이트로 설치합니다. 배포용 APK의 versionCode는 계속 증가시킵니다. 기존 앱 데이터는 유지하며, 저장 형식이 바뀌면 게임 코드에서 별도로 이전해야 합니다.

## 지금까지 삭제가 필요했던 이유

0.1.0~0.1.2는 매번 새 GitHub Actions 환경의 임시 키로 서명했습니다. 특히 공개된 0.1.2의 개인 서명 키를 보관하지 않아 그 APK 위에 새 키 APK를 업데이트할 수 없습니다. 고정 키 버전으로 전환할 때 한 번은 기존 앱을 삭제하고 설치해야 합니다. 그 이후에는 같은 키를 계속 사용합니다.

## 휴대폰에서 한 번만 설정

ChatGPT가 제공한 **RJM-Android-signing-setup.zip**은 개인 백업 파일입니다. 공개 GitHub 파일로 올리지 않습니다. 두 Secret 값도 채팅에 붙여 넣지 않습니다.

1. 휴대폰에서 ZIP을 내려받아 삼성 **내 파일**로 압축을 풉니다.
2. [새 Actions Secret 등록](https://github.com/GasArios/RJM_Android/settings/secrets/actions/new)을 엽니다.
3. Name은 `RJM_ANDROID_KEYSTORE_BASE64`, Secret은 ZIP 안의 `RJM_ANDROID_KEYSTORE_BASE64.txt` 전체 내용으로 입력하고 **Add secret**을 누릅니다.
4. 같은 주소를 다시 열어 Name은 `RJM_ANDROID_KEYSTORE_PASSWORD`, Secret은 `RJM_ANDROID_KEYSTORE_PASSWORD.txt` 전체 내용으로 입력하고 **Add secret**을 누릅니다.
5. 두 개를 등록했다고 ChatGPT에 알려주면 고정 키 빌드를 실행하고 확인합니다. [Actions 화면](https://github.com/GasArios/RJM_Android/actions/workflows/android.yml)의 **Run workflow**로 직접 실행할 수도 있습니다.

두 TXT는 각각 한 줄입니다. ZIP에는 암호화된 PKCS12 키 원본과 공개 인증서 지문도 있습니다. 키를 재생성하거나 다른 키로 Secrets를 덮어쓰지 않습니다. GitHub 연결에는 Secrets 등록 기능이 없어 이 한 번의 등록은 저장소 소유자가 직접 진행합니다.

## 구현

- `scripts/prepare_android_signing.py`: Secrets에서 키를 복원하고 별칭 `rjm`과 공개 인증서 지문을 확인합니다. 키를 600 권한으로 runner 임시 경로에만 쓰고, 사용 후 삭제합니다. 비밀값을 로그·소스·아티팩트로 출력하지 않습니다.
- `config/android-signing-certificate.sha256`: 사용할 고정 키의 **공개 인증서** SHA-256입니다. 개인 키가 아닙니다. 다른 키가 들어오면 발행을 중단합니다.
- `app/build.gradle`: 키·암호를 환경변수에서 읽어 같은 키로 서명합니다. `RJM_ANDROID_VERSION_CODE`를 읽습니다.
- `.github/workflows/android.yml`: versionCode는 `1000 + github.run_number`로 증가합니다. 두 Secret이 없으면 테스트용 임시 키로 검증만 진행하며 공개 릴리스를 만들지 않습니다. 하나만 있거나 키가 다르면 실패합니다.
- `scripts/verify_android_upgrade.py`: 이전·신규 APK의 패키지와 인증서가 같고 versionCode가 증가하는지 검증합니다. 실제 고정 키 배포에서는 APK 인증서도 고정 지문과 비교합니다.
- `tests/android-upgrade.sh`: 이전 APK를 설치하고 체크포인트·앱 내부 확인 파일을 만든 뒤 `adb install -r`로 새 APK를 덮어씁니다. 삭제 없이 파일·체크포인트가 보존되고 새 versionCode가 설치되는지 확인합니다.

테스트용 임시 키 APK는 설치용 릴리스가 아닙니다. Secret이 준비된 고정 키 빌드만 Releases에서 배포합니다. 기존 0.1.2 다운로드는 과거 검증 기록으로 남아 있습니다.

## 검증과 제한

로컬에서 백업 키와 공개 인증서 지문이 일치하는지 확인했습니다. [GitHub Actions 검증](https://github.com/GasArios/RJM_Android/actions/runs/43578188344)에서는 같은 테스트 키로 서명한 APK를 Android 15에서 1006 → 1007로 덮어썼습니다. 체크포인트와 앱 내부 파일이 그대로 남았고, 사격과 터치 조작 검증도 통과했습니다. 실제 배포에서는 등록한 고정 키로 같은 절차를 다시 검증합니다.

고정 키와 Play 배포용 키의 관리·백업은 지속적으로 필요합니다. 같은 키 업데이트가 가능하더라도 앱에서 의도적으로 데이터를 삭제하거나 저장 형식이 호환되지 않으면 데이터가 유지되지 않을 수 있으므로 저장 호환성은 후속 패치에서 검증합니다.

- [Android 공식 앱 서명 설명](https://developer.android.com/studio/publish/app-signing)
- [Android 공식 버전 관리 설명](https://developer.android.com/studio/publish/versioning)
- [GitHub Actions Secrets](https://docs.github.com/en/actions/how-tos/write-workflows/choose-what-workflows-do/use-secrets)
