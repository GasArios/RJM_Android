# RJM Godot Lab

Godot 4.6.3으로 만든 달빛 항구 첫 여정과 자유 시험장입니다. raylib 앱과는 다른 패키지 `com.gasarios.rjm.lab`으로, 설정과 설치 상태가 분리됩니다.

**[0.1.4 APK — 약 26 MB](https://github.com/GasArios/RJM_Android/releases/download/godot-lab-12/RJM-Godot-Lab.apk)** · [8방향 도트 시트](assets/recoil-cels.svg) · [검증 기록](VALIDATION.md)

기존 Godot Lab은 삭제하지 않고 업데이트할 수 있습니다. [이전 시험장 0.1.0](https://github.com/GasArios/RJM_Android/releases/tag/godot-lab-2)도 확인할 수 있습니다.

[플레이 방법](RELEASE.md) · [Godot Lab 빌드](https://github.com/GasArios/RJM_Android/actions/workflows/godot-lab.yml) · [Releases](https://github.com/GasArios/RJM_Android/releases)

| 파일 | 역할 |
|---|---|
| `scenes/lab.tscn` | 기존 시험장, 카메라, HUD |
| `scenes/harbor_terrain.tscn` / `scripts/draw_harbor_route.py` | 직접 편집할 수 있는 32px TileMapLayer 항구와 초기 배치 생성기 |
| `src/harbor_route.gd` / `src/harbor_waypoints.gd` | 단계 및 체크포인트, 조준 안내, 항구 소품 |
| `assets/harbor/` / `scripts/draw_harbor_art.py` | 직접 제작한 밤하늘·항구·소품과 다시 생성하는 스크립트 |
| `src/recoil_body.gd` | 8방향 반동·착지 도트 그림 선택과 총구 위치 |
| `assets/recoil-cels.svg` / `scripts/draw_recoil_sprite.py` | 28개의 고정 도트 그림과 생성 스크립트 |
| `src/tuning.gd` | 원본 총기·물리 수치를 픽셀과 초 단위로 관리 |
| `src/recoil.gd` / `src/field.gd` | 원본 Y-up 공식을 Godot Y-down으로 변환하고, X→Y 순서로 지형 충돌과 착지 보정 처리 |
| `src/touch.gd` / `src/loadout.gd` | 손가락 역할과 사격·자동 교체·지상 재장전 |
| `src/lab.gd` / `src/hud.gd` | 게임 흐름, 탄환 표적, 메뉴, 카메라, HUD와 별도 설정 |
| `tests/behavior.gd` | 입력·반동·충돌·재장전·일시정지 검증 |
| `tests/harbor_route.gd` | 터치로 여정 완주, 샷건 잠금과 저장·재시도 검증 |

원본과 같은 720 높이 UI와 60Hz 게임 스텝(플레이어 이동은 각 스텝을 두 번으로 분할)에서 비교합니다. 재장전은 해금된 총들의 원본 장전 시간 평균을 사용합니다. 항구에서는 리볼버로 시작하고, 보급함을 얻으면 샷건이 해금됩니다. 착지 체크포인트와 진행 단계가 자동 저장되며, `RETRY CHECKPOINT`는 진행을 유지합니다. 처음부터 다시 시작하려면 `REPLAY ROUTE`를 선택하세요.

PC에서 Godot 4.6.3으로 `project.godot`을 열면 씬을 편집할 수 있습니다. 플레이는 패드에 마우스를 누르고 드래그하며, Esc로 정지, R로 재장전, 1/2로 총 선택, 휠로 확대합니다. 휴대폰 테스트에 에디터 설치는 필요하지 않습니다.
