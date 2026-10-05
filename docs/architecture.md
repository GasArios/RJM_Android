# 첫 Android 프로토타입 구조

원본은 [GasArios/myFirstGame](https://github.com/GasArios/myFirstGame)의 `f0ba72fa2714ef30d17c9971fcc9a3d93abbbe1b` 커밋이다. 원본 C++ 헤더·소스 129개를 `game/`으로 옮겼다. 반동 물리, 높이에 따른 반동 감쇠, 타일 충돌, 적·투사체, 착지 복구와 LDtk 파서는 재사용한다.

## 한 프레임의 흐름

1. `RjmActivity.java`: 시스템 뒤로가기와 앱 중단을 C++에 알린다.
2. `Mobile/Platform.cpp`: raylib Android NDK 입력 큐에서 DOWN/MOVE/UP/CANCEL과 포인터 ID를 받아 저장한다. 짧은 탭도 보존한다.
3. `Core/InputState.cpp`: 실제 화면 좌표를 1280×720 가상 좌표로 바꿔 `Mobile/TouchControls.cpp`에 전달한다.
4. `TouchControls`: 터치 시작점에서 UI/사격/버티기 소유권을 정한다. 메뉴를 닫은 손가락은 떼기 전까지 사격으로 바뀌지 않는다.
5. `Scene/GameplayScene.cpp`: 총 발사와 원본 물리·전투를 진행한다. `Mobile/GameplayMobile.cpp`가 메뉴·시간 배율·체크포인트를 연결한다.
6. `Mobile/MobileAim.h`: 누른 순간과 의도적인 드래그 때만 월드 방향을 저장한다. 플레이어·카메라 이동만으로 반동 방향이 바뀌지 않는다.
7. `Mobile/ReloadQueue.h`: 예약한 총 슬롯을 유지한다. 착지 후 해당 총만 재장전하며 자동 무기 교체는 하지 않는다.

## 주요 파일과 수치

| 목적 | 파일 |
|---|---|
| 총 수치 | `game/src/Data/BuiltInData.cpp` |
| 원본 반동·중력·버티기 수치 | `game/include/RecoilJumpMan/Physics/RecoilMovementTuning.h` |
| 터치 영역·시간 배율 | `game/include/RecoilJumpMan/Mobile/TouchControls.h` |
| 드래그 10 가상 픽셀 문턱 | `game/src/Mobile/TouchControls.cpp` |
| 탄환 보정·반동 분리 | `game/src/Scene/GameplayScene.cpp` |
| APK 빌드·설치 검증 | `.github/workflows/android.yml`, `tests/android-smoke.sh` |

무기 창에서 모든 게임 시간은 8%로 진행한다. 입력·UI는 실시간이다. 일시정지·도움말에서는 게임 로직을 실행하지 않는다. 조준은 캐릭터 중심 8 월드 단위 이내면 사격하지 않는다. 탄환 보정은 원본 포인터 주변 후보 방식에 최대 6도 제한을 두며, 반동 방향은 보정하지 않는다. 자동 락온 입력은 없다.

빈 탄창 새 탭은 재장전 예약이다. 시간이 지나도 취소되지 않고 총을 바꿔도 처음 예약한 총에 남는다. 공중에서는 재장전 시간이 진행되지 않는다. 예약한 홀드는 떼기 전까지 추가 발사를 하지 않는다. 무기 창에서 선택 총의 부분 재장전도 예약할 수 있다.

## 체크포인트와 에셋

`checkpoint-v1.txt`를 앱 내부에 임시 파일 작성 후 원자적으로 교체한다. 3초마다, 일시정지 진입·저장 버튼·설정 변경 때 저장한다. 마지막 확인된 안전 위치·선택 총·탄환 보정 및 흔들림 설정을 복원한다. 앱을 새로 시작하면 체력·탄창·적은 초기화한다. 정식 RPG 세이브는 아니다.

원본 저장소에 실제 LDtk 맵·도트 이미지가 없어서 원본 코드의 디버그 월드를 사용한다. `game/assets/maps/starter_field.ldtk`를 추가하면 로더가 Android 에셋 API로 읽는다. 아직 도트 타일셋 렌더러는 없다. 스킬·아이템·배낭·보스·스토리·자이로는 이번 범위에 없다. 기능이 연결된 도움말·버티기·총 선택 버튼만 표시한다.

## 고정 빌드와 서명

raylib 5.5 커밋 `c1ab645ca298a2801097931d1079b10ff7eb9df8`, AGP 8.9.2, Gradle 8.11.1, JDK 17, SDK 35, NDK 27.2.12479018, CMake 3.22.1을 사용한다. raylib Android 입력 콜백에 작은 CMake 훅을 넣으며, 버전과 맞지 않으면 빌드를 실패시킨다.

GitHub Actions에서 `gradle --no-daemon :app:assembleDebug --stacktrace`를 실행한다. 출력은 `app/build/outputs/apk/debug/app-debug.apk`이다. APK에 Galaxy용 `arm64-v8a`와 에뮬레이터용 `x86_64`를 넣는다.

개인 서명 키는 저장소에 올리지 않는다. Gradle이 임시 빌드 환경에서 테스트 키를 만든다. 다른 빌드의 키가 달라지면 기존 테스트 앱을 삭제한 후 재설치해야 하며 앱 내부 체크포인트가 지워진다. 후속 개발에서는 GitHub Secrets에 보관한 지속 서명 키를 별도로 설정한다. 앱은 네트워크·저장소 접근 권한을 요청하지 않는다.

## 검증의 경계

자동 테스트는 터치 소유권·짧은 탭·메뉴 입력 차단·재장전 예약·반동 분리·타일 착지와 Android 설치·기동을 검사한다. 실제 S24 Ultra 손가락 조작감, 시스템 뒤로가기 제스처, 장시간 발열·배터리·프레임 안정성은 실기기에서 확인해야 한다.
