#pragma once

// AimAssistResolver.h
// - 마우스 포인터 근처의 적을 찾아 이번 발사의 실제 목표 좌표를 보정합니다.
// - 마우스 위치 자체는 움직이지 않고, 발사/프리뷰에 사용할 목표 좌표만 해석합니다.
// - 이 클래스는 후보 선택만 담당하며, 탄 생성이나 반동 적용은 Player/Gun 쪽에서 처리합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

#include <cstddef>
#include <vector>

namespace rjm
{
    struct AimAssistTuning
    {
        // enabled:
        // - false이면 후보가 있어도 조준 보정을 사용하지 않습니다.
        bool enabled = true;

        // screenRadiusPixels:
        // - 마우스 포인터 기준 보정 반경입니다.
        // - 화면 픽셀 기준이라 카메라 줌이 달라져도 플레이어가 느끼는 보정 범위가 일정합니다.
        float screenRadiusPixels = 42.0f;

        // maxCorrectionDegrees:
        // - 원래 마우스 조준 방향과 보정 대상 방향 사이의 최대 허용 각도입니다.
        // - 너무 큰 각도 보정은 이동 의도를 흔들 수 있으므로 제한합니다.
        float maxCorrectionDegrees = 15.0f;

        // radiusScale:
        // - 접근성 옵션이나 난이도 옵션에서 전체 보정 반경을 키우거나 줄일 때 사용할 배율입니다.
        float radiusScale = 1.0f;

        // useAssistedTargetForRecoil:
        // - true이면 탄 방향과 반동 방향이 모두 보정 대상 기준으로 계산됩니다.
        // - false로 바꾸면 탄은 적에게 보정하되, 반동은 원래 마우스 방향을 유지할 수 있습니다.
        bool useAssistedTargetForRecoil = true;

        // usePredictedTarget:
        // - true이면 후보가 가진 예측 좌표를 탄환 목표로 사용할 수 있습니다.
        // - 좌클릭 일반 보정에서는 약하게 적용해 플레이어의 예측 사격 의도를 덜 방해합니다.
        bool usePredictedTarget = false;
    };

    struct AimAssistCandidate
    {
        // targetId:
        // - 조준 보정 대상의 데이터 id입니다.
        // - 일반 적, 보스 약점, 파괴 가능한 오브젝트를 구분하는 데 사용할 수 있습니다.
        DefinitionId targetId;

        // worldPosition:
        // - 실제 탄이 향할 월드 좌표입니다.
        // - 현재 일반 적은 중심점을 쓰고, 나중에 보스 약점 좌표로 확장할 수 있습니다.
        Vector2 worldPosition = { 0.0f, 0.0f };

        // screenPosition:
        // - worldPosition을 카메라로 변환한 가상 화면 좌표입니다.
        // - 마우스 포인터와 가까운지 판단하는 기준입니다.
        Vector2 screenPosition = { 0.0f, 0.0f };

        // screenRadiusPixels:
        // - 이 후보가 사용할 개별 보정 반경입니다.
        // - 0 이하이면 AimAssistTuning의 기본 반경을 사용합니다.
        float screenRadiusPixels = 0.0f;

        // priority:
        // - 같은 거리라면 priority가 높은 후보를 더 선호합니다.
        // - 보스 약점, 중요한 타깃, 큰 적 중심점 등에 가중치를 줄 수 있습니다.
        float priority = 1.0f;

        // enabled:
        // - 보스 페이즈, 무적 상태, 은신 상태처럼 일시적으로 조준 보정 후보에서 제외할 때 사용합니다.
        bool enabled = true;

        // stableId:
        // - 후보를 프레임 사이에서 식별하기 위한 값입니다.
        // - 현재는 EnemyPool의 enemy index를 넣고, 나중에는 EntityId 같은 고유 id로 바꿀 수 있습니다.
        std::size_t stableId = 0;

        // hasStableId:
        // - stableId가 의미 있는 값인지 나타냅니다.
        // - 파괴 오브젝트나 임시 약점처럼 아직 고유 id가 없는 후보도 같은 구조를 쓸 수 있게 둡니다.
        bool hasStableId = false;

        // hasLineOfSight:
        // - 플레이어와 후보 사이가 고체 지형에 막히지 않았는지 나타냅니다.
        // - 마우스 근처 약한 보정은 이 값을 강하게 요구하지 않지만, 전투 락온은 이 값을 우선적으로 봅니다.
        bool hasLineOfSight = true;

        // worldVelocity:
        // - 움직이는 적에게 최소한의 선형 예측 사격을 적용하기 위한 월드 속도입니다.
        Vector2 worldVelocity = { 0.0f, 0.0f };

        // predictedWorldPosition:
        // - 탄속과 후보 속도를 바탕으로 계산한 약한 미래 조준점입니다.
        // - targetWorldPosition은 여전히 실제 대상 위치로 두고, projectileWorldTarget만 이 값을 쓸 수 있습니다.
        Vector2 predictedWorldPosition = { 0.0f, 0.0f };

        // predictedScreenPosition:
        // - predictedWorldPosition을 화면 좌표로 변환한 값입니다.
        // - 플레이어가 적 앞쪽을 조준했을 때 보정 후보 선택에 사용합니다.
        Vector2 predictedScreenPosition = { 0.0f, 0.0f };

        // predictedHasLineOfSight:
        // - 예측 조준점까지의 시야선입니다.
        // - 예측점이 벽 뒤라면 현재 위치로 fallback할 수 있게 합니다.
        bool predictedHasLineOfSight = true;

        // hasPredictedWorldPosition:
        // - predictedWorldPosition이 이번 무기/모드 기준으로 의미 있게 계산되었는지 나타냅니다.
        bool hasPredictedWorldPosition = false;
    };

    struct AimAssistResult
    {
        // assisted:
        // - 이번 조준에 보정이 실제로 적용되었는지 나타냅니다.
        bool assisted = false;

        // lockOn:
        // - 우클릭 전투 락온 모드로 선택된 결과인지 나타냅니다.
        // - 일반 마우스 근처 보정과 렌더링 색/표시 방식을 구분할 때 사용합니다.
        bool lockOn = false;

        // targetId:
        // - 보정이 걸린 대상 id입니다.
        DefinitionId targetId;

        // rawWorldTarget:
        // - 마우스 포인터가 원래 가리키던 월드 좌표입니다.
        Vector2 rawWorldTarget = { 0.0f, 0.0f };

        // projectileWorldTarget:
        // - 탄환 방향 계산에 사용할 월드 좌표입니다.
        Vector2 projectileWorldTarget = { 0.0f, 0.0f };

        // recoilWorldTarget:
        // - 반동 방향 계산에 사용할 월드 좌표입니다.
        // - 현재 기본값은 projectileWorldTarget과 같지만, 탄 보정/반동 보정을 분리할 수 있게 따로 둡니다.
        Vector2 recoilWorldTarget = { 0.0f, 0.0f };

        // targetScreenPosition:
        // - 보정 대상의 화면 좌표입니다.
        // - UI 표시나 디버그 표시에서 사용할 수 있습니다.
        Vector2 targetScreenPosition = { 0.0f, 0.0f };

        // targetWorldPosition:
        // - 보정 대상의 월드 좌표입니다.
        // - 타깃 링이나 약점 표시를 그릴 때 사용할 수 있습니다.
        Vector2 targetWorldPosition = { 0.0f, 0.0f };

        // targetStableId:
        // - 락온 유지용 대상 식별자입니다.
        // - hasTargetStableId가 false이면 이 값은 의미가 없습니다.
        std::size_t targetStableId = 0;

        // hasTargetStableId:
        // - targetStableId가 의미 있는지 나타냅니다.
        bool hasTargetStableId = false;

        // hasLineOfSight:
        // - 선택된 대상까지 시야선이 열려 있는지 나타냅니다.
        // - 렌더러는 이 값으로 락온 선의 색을 바꾸거나 끊긴 느낌을 줄 수 있습니다.
        bool hasLineOfSight = true;

        // screenDistancePixels:
        // - 마우스 포인터와 보정 대상 사이의 화면 거리입니다.
        float screenDistancePixels = 0.0f;

        // correctionDegrees:
        // - 원래 조준 방향에서 보정 대상 방향으로 얼마나 꺾였는지 나타냅니다.
        float correctionDegrees = 0.0f;
    };

    class AimAssistResolver
    {
    public:
        // Resolve:
        // - 플레이어 위치, 원래 마우스 목표, 마우스 화면 좌표, 후보 목록을 받아 이번 발사의 목표를 결정합니다.
        // - 후보가 없거나 조건을 통과하지 못하면 rawWorldTarget 그대로 반환합니다.
        AimAssistResult Resolve(
            Vector2 playerWorldPosition,
            Vector2 rawWorldTarget,
            Vector2 mouseScreenPosition,
            const std::vector<AimAssistCandidate>& candidates,
            const AimAssistTuning& tuning) const;

    private:
        static AimAssistResult BuildRawResult(Vector2 rawWorldTarget);
    };
}

