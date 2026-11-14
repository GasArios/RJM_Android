# RJM Godot Lab 0.1.0 검증

- [빌드와 Android 15 테스트](https://github.com/GasArios/RJM_Android/actions/runs/43581835372)
- [ARM64 릴리스](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-2)
- 빌드 소스: `b7ca6f7610b16f00b5e6a1767d5380e7092ed707`, Godot 4.6.3
- APK: `com.gasarios.rjm.lab`, versionCode 2, 25,956,282 bytes
- APK SHA-256: `fa5a5891dba06c81b289401c62752d502510cbda1d6d796fe13fb8d30e524dad`

Godot 자동 테스트에서 발판의 실제 TileMapLayer 셀, 바닥·벽 충돌, 반동 부호, 지상 버티기, 패드 중심·홀드·짧은 탭·핀치와 손가락 소유권, 탄창, 재장전 예약·완료, 일시정지를 확인했습니다. 동일한 바닥에서 아래로 한 발, 60Hz에 두 이동 하위 스텝을 적용한 기준에서 원본 C++ 계산은 리볼버 75.64px/샷건 164.44px, Godot HUD 반올림은 각각 76px/164px였습니다. 발판 형상과 화면 연출은 시험판용이며, 이 수치가 전체 조작감 일치를 보장하지 않습니다.

APK 패키지명, ARM64 및 별도 x86_64 빌드, 서명 인증서, 16KB 정렬을 검사했습니다. Android 15 x86_64 에뮬레이터에서 실제 터치로 일반 화면/패드 중심 오발 방지, 리볼버·샷건 발사, GUNS 메뉴, 전체 재장전, Android 뒤로가기, 앱 전환 일시정지와 프로세스 재시작 후 조작 설정 복원을 통과했습니다. Galaxy의 UI 감각, 입력 지연, 발열과 성능은 실기기 비교가 남아 있습니다.

`main`의 raylib 앱과 기존 업데이트 경로는 그대로입니다. 이번 APK는 별도 설치되며 두 앱의 저장 데이터를 공유하지 않습니다.
