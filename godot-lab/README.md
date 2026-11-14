# RJM Godot Lab

기존 raylib 앱의 조작감을 Godot 4.6.3의 작은 시험장에서 비교하는 프로젝트입니다. 기존 앱과는 다른 패키지 `com.gasarios.rjm.lab`으로, 설정과 설치 상태가 분리됩니다. 기존 APK나 저장 데이터를 지우지 않습니다.

**[Galaxy용 APK 다운로드 — 약 26 MB](https://github.com/GasArios/RJM_Android/releases/download/godot-lab-2/RJM-Godot-Lab.apk)** · [릴리스](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-2) · [검증 기록](VALIDATION.md)

[플레이 방법](RELEASE.md) · [Godot Lab 빌드](https://github.com/GasArios/RJM_Android/actions/workflows/godot-lab.yml) · [Releases](https://github.com/GasArios/RJM_Android/releases)

| 파일 | 역할 |
|---|---|
| `scenes/lab.tscn` | 직접 편집할 수 있는 32px TileMapLayer 시험장, 카메라, HUD |
| `src/tuning.gd` | 원본 총기·물리 수치를 픽셀과 초 단위로 관리 |
| `src/recoil.gd` / `src/field.gd` | 원본 Y-up 공식을 Godot Y-down으로 변환하고, X→Y 순서로 지형 충돌과 착지 보정 처리 |
| `src/touch.gd` / `src/loadout.gd` | 손가락 역할과 사격·자동 교체·지상 재장전 |
| `src/lab.gd` / `src/hud.gd` | 게임 흐름, 탄환 표적, 메뉴, 카메라, HUD와 별도 설정 |
| `tests/behavior.gd` | 입력·반동·충돌·재장전·일시정지 검증 |

원본과 같은 720 높이 UI와 60Hz 게임 스텝(플레이어 이동은 각 스텝을 두 번으로 분할)에서 비교합니다. 두 총의 원본 장전 시간 평균을 쓰므로, 3종을 모두 드는 raylib 시험장과 총 장전 시간은 똑같지 않습니다. 이번 범위는 두 총, 고정 패드/터치 원, 발판과 표적입니다. 캐릭터/화면 중앙 조준, SMG, LDtk 로딩, 적/보스 AI, 기존 체크포인트 이전은 포함하지 않습니다.

PC에서 Godot 4.6.3으로 `project.godot`을 열면 씬을 편집할 수 있습니다. 플레이는 패드에 마우스를 누르고 드래그하며, Esc로 정지, R로 재장전, 1/2로 총 선택, 휠로 확대합니다. 휴대폰 테스트에 에디터 설치는 필요하지 않습니다.
