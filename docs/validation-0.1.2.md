# 0.1.2 검증 기록

검증일: 2026-10-05. 소스 커밋: `dd2cad24b35303bc7167dbbd5623bb9b8c8dce60`.

- [GitHub Actions 실행](https://github.com/GasArios/RJM_Android/actions/runs/37265956530): build·smoke·release 모두 성공.
- [릴리스](https://github.com/GasArios/RJM_Android/releases/tag/android-prototype-6)
- [APK 다운로드](https://github.com/GasArios/RJM_Android/releases/download/android-prototype-6/RJM-Android-0.1.2.apk)

## 로컬·CI 행동 검증

`bash tests/run-core-tests.sh <raylib 헤더 폴더>`를 로컬과 CI에서 통과했습니다. 기존 짧은 탭, 버티기와 사격 포인터 분리, 메뉴·앱 중단 취소, 핀치·카메라·두 조준 기준·전체 재장전 수식·원본 자동 교체·반동·타일 착지를 유지합니다.

추가 검증:

- 3·4번은 패드 밖 화면 탭으로 발사하지 않음. 밖에서 시작한 손가락이 패드로 이동해도 사격 포인터가 되지 않음.
- 작은 중심 영역에서는 정지, 바깥 이동으로 사격 시작, 유지 입력 지속, 중심 복귀 정지, 실제 손 떼기 시 작은 원 중앙 복귀.
- 원 밖 드래그 유지와 작은 원의 시각적 제한. 당긴 거리에 상관없는 단위 조준 방향. 캐릭터 위치에 영향받지 않는 고정 중심.
- 패드+BRACE와 두 패드 손가락이 핀치로 바뀌지 않음. 패드 손가락이 왼쪽 버튼 위로 이동해도 클릭하지 않음.
- 왼쪽 RELOAD가 사격을 차단하고 중심 왕복으로 재입력되지 않음. 모드·메뉴·화면 크기 변경과 화면 밖 이탈 시 이전 홀드로 발사하지 않음.
- 패드 밖 두 손가락 핀치 유지, 남은 손가락이 사격 포인터로 바뀌지 않음. 4→1 모드 순환.
- 반자동 유지 사격은 실제 총의 0.3초 쿨타임을 지킴. 자동 반자동 총 교체 후 홀드가 이어짐.
- 모든 탄 소진 시 전체 재장전 예약. 공중에서 대기, 지상 장전 완료 후에도 손을 떼기 전에는 추가 발사하지 않음.

전체 C++ 파일의 데스크톱 구문 검증, NDK 플랫폼 파일을 제외한 Android 조건부 구문 검증도 통과했습니다. 실제 NDK 플랫폼 파일은 아래 Android 빌드에서 컴파일했습니다.

## Android 빌드·실행

JDK 17 / AGP 8.9.2 / Gradle 8.11.1 / SDK 35 / NDK 27.2.12479018 / CMake 3.22.1, raylib 고정 커밋을 사용했습니다. `assembleDebug`와 별도 입력 검증용 `assembleDebugAndroidTest`가 성공했습니다. 게임 APK에 테스트 입력 주입 코드를 포함하지 않습니다.

Android 15 x86_64 에뮬레이터에서 설치·네이티브 실행·실제 터치 사격·무기 선택·뒤로가기·홈/복귀 일시정지와 다음을 검증했습니다.

- 일시정지에서 실제 다중 포인터 입력으로 배율을 약 1.10→1.26으로 확대하며 오발하지 않음.
- CONTROL의 네 가지 모드 선택·저장.
- 3·4번 각각 화면 탭 무시, 중심 정지, 리볼버 유지 입력의 반복 사격, 중심 복귀 정지, 바깥 재진입 재개.
- 3번에서 오른손 홀드 중 왼쪽 전체 RELOAD 입력, 장전 뒤 추가 발사 없음, 중심 왕복으로 차단이 풀리지 않음.
- 두 모드의 실제 손 떼기 후 정지, 새 짧은 탭은 정확히 한 발, 이후 반복 없음.
- 프로세스 종료/재기동 후 저장된 4번 모드(enum 3)와 배율 1.10 복원. 테스트 필드 재시작은 기존 규칙대로 배율을 1.10으로 초기화하므로 앞선 핀치의 1.26과 최종 복원값은 다릅니다.
- 시작·복귀·재기동 로그에서 Fatal signal / FATAL EXCEPTION 없음.

실행 결과: `PASS native two-finger pinch, four modes, pad hold/neutral/release, reload gate and saved aim/zoom`.

Actions의 `Android-smoke-evidence`에서 3·4번 홀드/손 떼기, 재장전 차단, 네 가지 모드 설정, 핀치 설정 화면을 확인했습니다. 3번 작은 원의 이동·중앙 복귀와 4번 작은 원 없음, 왼쪽 RELOAD 배치를 확인했습니다. 에뮬레이터의 화면 FPS는 실제 Galaxy 성능 측정값이 아닙니다.

## APK 무결성·설치

- 파일 크기: 6,465,668 bytes (약 6.5MB)
- SHA-256: `b8f3584867f7670c052f6601ed4af2a477ef581e223639fea8e6d8e785b03bcf`
- APK 공개 릴리스의 digest·크기와 내려받은 CI 아티팩트가 일치하며 `SHA256SUMS.txt`와도 일치합니다.
- versionCode 3 / versionName 0.1.2, 패키지 `com.gasarios.rjm`, ARM64와 x86_64 포함.
- CI `apksigner verify --verbose`: APK Signature Scheme v2 검증 성공.
- ARM64 `librjm.so`와 `libc++_shared.so`의 모든 LOAD 세그먼트 정렬: 16,384 bytes.
- 공개 인증서 SHA-256: `d7ce61495d01d3d098e1de47b484fa8f9f23e58b997ef43f4fe06aaced1b3200`.
- 기존 0.1.1과 서명이 다릅니다. **기존 앱 삭제 후 설치**하며 앱 내부 체크포인트가 초기화됩니다. 개인 서명 키는 공개하지 않았습니다.

Galaxy S24 Ultra의 손가락 가림·크기·정밀 조준·피로·발열·장시간 성능은 실기기에서 비교해야 합니다. 이번 패치는 스킬·아이템·자이로를 추가하지 않습니다.
