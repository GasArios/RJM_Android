#pragma once

// RecoilMovementTuning.h
// - 반동 이동의 조작감을 조절하는 상수들을 한곳에 모읍니다.
// - 나중에 밸런스 튜닝, 난이도 옵션, 장비/인챈트 보정이 들어와도
//   물리 공식 자체를 고치지 않고 이 값들을 바꾸는 방식으로 조절할 수 있습니다.

namespace rjm
{
    struct RecoilMovementTuning
    {
        // 정점 체공 구간을 판단하는 y속도 임계값 H입니다.
        // y-up 물리 좌표계 기준이므로, +는 상승, -는 하강입니다.
        float apexThreshold = 120.0f;

        // 초당 기본 중력입니다.
        // 매 프레임 velocity.y에서 gravity * dt만큼 뺍니다.
        float gravity = 1800.0f;

        // 강한 상승 구간에서 1초 뒤 속도가 얼마나 남는지 나타내는 지수 감속 계수입니다.
        // 0과 1 사이 값이어야 하며, 낮을수록 상승 속도를 더 강하게 깎습니다.
        float upwardDragPerSecond = 0.22f;

        // 정점 근처에서는 중력을 약하게 적용해 조준할 시간을 줍니다.
        float apexGravityScale = 0.55f;

        // 공중 수평 감속 계수입니다.
        // 1초 뒤 수평 속도가 airDragPerSecond 비율만큼 남는다고 생각하면 됩니다.
        float airDragPerSecond = 0.25f;

        // 지상 수평 마찰 계수입니다.
        // 공중보다 훨씬 작게 두면 땅에서 빠르게 멈춥니다.
        float groundFrictionPerSecond = 0.035f;

        // 버티기 중 수평 반동에 곱할 비율입니다.
        // 0.2라면 발사 순간 좌우 반동의 20%만 실제 속도에 반영됩니다.
        float braceHorizontalRecoilScale = 0.2f;

        // 버티기 중 위로 뜨는 반동에 곱할 비율입니다.
        // 아래쪽 적을 쏠 때 플레이어가 너무 쉽게 떠버리는 것을 막습니다.
        float braceUpwardRecoilScale = 0.12f;

        // 버티기 중 아래로 누르는 반동에 곱할 비율입니다.
        // 지상에서 바닥 쪽 힘은 바닥이 받아내므로 기본적으로 속도로 거의 반영하지 않습니다.
        float braceDownwardRecoilScale = 0.0f;

        // 버티기 중 이 값 이하의 상승 반동은 바닥에 앵커링된 것으로 보고 완전히 흡수합니다.
        // 리볼버는 고정되고, 샷건/핸드캐논은 힘이 남으면 살짝 뜨게 만들기 위한 문턱값입니다.
        float braceAnchoredRiseSpeed = 90.0f;

        // 버티기를 누르고 지상에 있는 동안 사용할 추가 수평 마찰입니다.
        // 발사 순간을 버텨낸 뒤 남은 수평 속도를 빠르게 죽입니다.
        float braceGroundFrictionPerSecond = 0.0015f;

        // 버티기 중 수평 속도가 이 값보다 작아지면 0으로 스냅합니다.
        float braceHorizontalSnapEpsilon = 14.0f;

        // 발사 순간 y축 시너지 보너스 비율입니다.
        // 네가 제안한 수식의 0.2 부분입니다.
        float verticalRecoilSynergyScale = 0.2f;

        // 공중에서 너무 높이 올라갔을 때 위쪽 반동 효율을 점진적으로 낮출지 여부입니다.
        // 단발 상승감은 살리고, 높은 고도에서 계속 총을 연타해 무한 체공하는 플레이를 줄이기 위한 장치입니다.
        bool airborneUpwardRecoilHeightDampingEnabled = true;

        // 체공 시작 지점 대비 이 높이까지는 위쪽 반동을 100% 적용합니다.
        // 초반 점프감과 낮은 고도 전투 템포를 보존하기 위한 안전 구간입니다.
        float airborneUpwardRecoilFullEfficiencyHeight = 400.0f;

        // 체공 시작 지점 대비 500px 부근에서는 위쪽 반동 효율을 약 73%까지 낮춥니다.
        // 400~500 사이에서는 100%에서 이 값까지 선형으로 부드럽게 이어집니다.
        float airborneUpwardRecoilFirstDampingHeight = 500.0f;
        float airborneUpwardRecoilFirstDampingScale = 0.73f;

        // 650px 부근에서는 위쪽 반동 효율을 약 53%까지 낮춥니다.
        // 이 구간부터는 "더 올라갈 수는 있지만 공짜로 계속 뜨지는 않는" 체감이 나야 합니다.
        float airborneUpwardRecoilSecondDampingHeight = 650.0f;
        float airborneUpwardRecoilSecondDampingScale = 0.53f;

        // 800px 부근에서는 위쪽 반동 효율을 약 38%까지 낮춥니다.
        // 높은 고도에서 전투가 정적으로 굳는 문제를 줄이는 핵심 구간입니다.
        float airborneUpwardRecoilThirdDampingHeight = 800.0f;
        float airborneUpwardRecoilThirdDampingScale = 0.38f;

        // 900px 이상에서는 위쪽 반동 효율을 최소값 근처로 제한합니다.
        // 완전히 0으로 만들지 않는 이유는 높은 곳에서도 마지막 복구 조작의 여지를 남기기 위해서입니다.
        float airborneUpwardRecoilMinimumHeight = 900.0f;
        float airborneUpwardRecoilMinimumScale = 0.30f;

        // 무기별 고도 감쇠 저항이 상승 높이 감쇠 곡선을 얼마나 늦추는지 나타냅니다.
        // 예: 값이 0.75이고 무기 저항이 1이면, climbedHeight를 1.75로 나눈 것처럼 취급합니다.
        float airborneUpwardRecoilResistanceHeightScale = 0.75f;

        // 상승 반동으로 도달할 수 있는 최대 상승 속도입니다.
        float maxRiseSpeed = 2000.0f;

        // 일반 낙하에서 허용되는 최대 낙하 속도입니다.
        float maxFallSpeed = 2000.0f;

        // 아래 방향 반동, 즉 Stomp 계열 충격으로 허용되는 최대 하강 속도입니다.
        // 일반 낙하보다 조금 크게 둬서 "찍어 누르는" 맛을 살립니다.
        float maxStompSpeed = 3000.0f;

        // 수평 반동으로 도달할 수 있는 최대 수평 속도입니다.
        float maxHorizontalSpeed = 3000.0f;

        // x축 속도가 이 값보다 작아지면 0으로 스냅합니다.
        // 지수 감속이 영원히 0에 가까워지기만 하는 문제를 막습니다.
        float horizontalSnapEpsilon = 3.0f;
    };
}

