# 달빛 항구 첫 여정 0.1.4 검증

- [빌드·Android 15·배포 검증](https://github.com/GasArios/RJM_Android/actions/runs/43667768111)과 [공개 APK](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-12). ARM64, versionCode 12, 26,007,624 bytes. APK SHA-256: `3f83d32c3d85301c27436e36009bb03445b89ab5e8aaf80073b3bed6d2e77055`. 기존 Godot Lab과 같은 서명 인증서입니다.
- Godot 자동 플레이가 우측 패드의 금빛 안내를 11번 따라 눌러 등대를 밝혔습니다. 플레이어 좌표를 강제로 이동시키지 않았습니다. 체크포인트, 총기 잠금과 후방 낙하 시 안내, 진행 초기화 및 기존 시험장 320셀 유지도 확인했습니다.
- Android 15 에뮬레이터에서 터치로 첫 발사, 낮은 부두, 보급함 해금, 샷건으로 높은 부두 착지, 프로세스 종료 후 진행 복원을 검증했습니다. 기존 시험장의 오발 방지, 무기 선택, 전체 재장전, 뒤로가기, 앱 전환 일시정지와 조작 설정 복원도 통과했습니다.
- 2400×1080 인게임 캡처에서 시작 화면, 샷건 해금 및 높은 부두의 목표와 조준 표시를 확인했습니다. 갤럭시 조작감은 실기기 테스트가 남아 있습니다.

# 8방향 도트 반동 0.1.3 검증

- [최종 빌드 및 Android 15 재검증](https://github.com/GasArios/RJM_Android/actions/runs/43657685523)과 [배포 APK](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-9). ARM64, versionCode 9, 25,982,516 bytes. 공개 파일의 SHA-256 `62d7c8243d6e88cdf3d378807db911f3f9fde2ae31068fafdb8e5c110dde064c`가 CI 결과와 일치하며, 이전 0.1.2와 같은 서명 인증서입니다.
- [발사 프레임을 직접 확인한 Godot 및 Android 15 검토 빌드](https://github.com/GasArios/RJM_Android/actions/runs/43657340400)
- 8방향의 눌림·늘어남·복귀 그림과 정지·착지 그림까지 28장입니다. `scripts/draw_recoil_sprite.py`로 SVG 시트와 총구 위치를 찾는 픽셀 데이터를 함께 생성합니다.
- 실행 중에는 몸 이미지를 회전하거나 보간하지 않습니다. 실제 속도 변화에서 발사마다 방향 프레임을 고르고, 총구·탄환과 물리는 연속 방향을 유지합니다.
- 자동 검사로 8방향, 한 방향 내부의 각도 차이, 샷건 유지 시간, 착지 시 아랫변, 일시정지, 프레임 복귀를 확인했습니다. 리볼버 76px, 샷건 164px 상승 높이와 320개 발판 좌표를 유지했습니다.
- Android 캡처에서 정지한 사각형과 발사 후 계단형 외곽선을 확인했습니다. 설치·사격·재장전·뒤로가기·설정 복원도 통과했습니다. 갤럭시에서의 움직임 체감은 실기기 테스트로 확인합니다.

# 캐릭터 반동 애니메이션 0.1.2 검증

- [최종 빌드 및 Android 15 재검증](https://github.com/GasArios/RJM_Android/actions/runs/43591009601)과 [배포 APK](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-7). ARM64, versionCode 7, 25,978,059 bytes. 공개 파일의 SHA-256 `2dca3de2c49b5d1a02717ed5cc2281c2a65b6515fe81fa2c5118d51e8894a938`가 빌드 결과와 일치합니다.
- [Godot 및 Android 15 검증](https://github.com/GasArios/RJM_Android/actions/runs/43590649392)
- 정지 시 정사각형, 발사 직후 압축, 대각선 반동 방향으로 늘어남, 총구 위치 추적, 착지 시 바닥 접촉, 정사각형 복귀와 일시정지 중 애니메이션 멈춤을 확인했습니다.
- 리볼버 76px, 샷건 164px의 기존 최고 상승 높이와 충돌 셀 320개의 위치를 유지했습니다.
- Android 에뮬레이터에서 정지 상태와 발사 후 실제 화면을 열어 눈 제거와 세로 변형을 확인했습니다. 갤럭시에서의 애니메이션 체감은 사용자 실기기 테스트가 남아 있습니다.

# 달빛 항구 0.1.1 그래픽 검증

- [배포 APK](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-5), Android versionCode 5, ARM64, 25,982,006 bytes. SHA-256: `67ceb2196455e4e6f076d3705f32cc9be9f95d6ab7bc1305e3304657663da9f2`.
- [최종 빌드와 Android 15 재검증](https://github.com/GasArios/RJM_Android/actions/runs/43589498390). 공개 APK·화면 캡처 체크섬이 CI 결과와 일치합니다.
- [Android 15 렌더링 및 조작 검증](https://github.com/GasArios/RJM_Android/actions/runs/43589111282)
- `range.png`, `recoil.png`, `guns.png`, `pause.png`의 2400×1080 인게임 캡처를 직접 확인했습니다. 배경과 파란 주인공, 실제 충돌이 있는 돌 윗면을 구분할 수 있었고 메뉴 글자와 터치 조작이 겹치지 않았습니다.
- 기존 소프트웨어 그래픽 설정은 입력 검사가 통과해도 빈 화면만 그렸습니다. 에뮬레이터를 ANGLE로 실행하고 캡처의 픽셀 변화를 확인하여 빈 화면이 통과하지 않게 했습니다. 렌더링, 설치, 사격, 총 교체, 재장전, 뒤로가기, 설정 복원 검증을 다시 통과했습니다.
- 원래 지형의 320개 충돌 셀 좌표와 발사·조작 규칙을 유지했습니다. 새 그림은 `scripts/draw_harbor_art.py`로 다시 생성할 수 있습니다.
- Galaxy S24 Ultra의 화면 밝기에서 보이는 착지선과 연사 중 가독성은 사용자 실기기 테스트가 남아 있습니다.

# RJM Godot Lab 0.1.0 검증

- [빌드와 Android 15 테스트](https://github.com/GasArios/RJM_Android/actions/runs/43581835372)
- [ARM64 릴리스](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-2)
- 빌드 소스: `b7ca6f7610b16f00b5e6a1767d5380e7092ed707`, Godot 4.6.3
- APK: `com.gasarios.rjm.lab`, versionCode 2, 25,956,282 bytes
- APK SHA-256: `fa5a5891dba06c81b289401c62752d502510cbda1d6d796fe13fb8d30e524dad`

Godot 자동 테스트에서 발판의 실제 TileMapLayer 셀, 바닥·벽 충돌, 반동 부호, 지상 버티기, 패드 중심·홀드·짧은 탭·핀치와 손가락 소유권, 탄창, 재장전 예약·완료, 일시정지를 확인했습니다. 동일한 바닥에서 아래로 한 발, 60Hz에 두 이동 하위 스텝을 적용한 기준에서 원본 C++ 계산은 리볼버 75.64px/샷건 164.44px, Godot HUD 반올림은 각각 76px/164px였습니다. 발판 형상과 화면 연출은 시험판용이며, 이 수치가 전체 조작감 일치를 보장하지 않습니다.

APK 패키지명, ARM64 및 별도 x86_64 빌드, 서명 인증서, 16KB 정렬을 검사했습니다. Android 15 x86_64 에뮬레이터에서 실제 터치로 일반 화면/패드 중심 오발 방지, 리볼버·샷건 발사, GUNS 메뉴, 전체 재장전, Android 뒤로가기, 앱 전환 일시정지와 프로세스 재시작 후 조작 설정 복원을 통과했습니다. Galaxy의 UI 감각, 입력 지연, 발열과 성능은 실기기 비교가 남아 있습니다.

`main`의 raylib 앱과 기존 업데이트 경로는 그대로입니다. 이번 APK는 별도 설치되며 두 앱의 저장 데이터를 공유하지 않습니다.
