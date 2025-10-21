#pragma once
#include "../IUpdateSystem.h"
#include <vector>
#include <DirectXMath.h>
#include "../Physics/ObbUtil.h"

/**
 * @file CollisionSystemOBB.h
 * @brief 動的OBBを静的OBBに対して「スイープ（連続判定）」し、接触手前で停止＋スライドするSystem
 * @details
 * - 入力：Transform, Rigidbody, MotionDelta, ObbCollider（動的）／ Transform, ObbCollider（静的）
 * - 手順：
 *   1) 静的OBBを事前収集（キャッシュ）
 *   2) 各動的に対し、remain(希望移動量) を持ってスイープ前進→ヒット時は接触直前へ移動
 *   3) 法線方向の「壁に向かう速度成分だけ」を除去し、残り移動を法線除去してスライド
 *   4) groundNormalY 閾値で接地判定を更新
 *   5) 最終的に md.delta を消費し尽くす
 */
class CollisionSystemOBB final : public IUpdateSystem
{
public:
    /** @brief 反復回数やスキン幅などの既定値を設定 */
    explicit CollisionSystemOBB(int sweepIterations, float sweepSlop)
        : m_sweepIterations(sweepIterations)
        , m_sweepSlop(sweepSlop)
        , m_skin(0.01f)
        , m_groundNormalY(0.5f)
    {
    }

    void Update(class World& world, float dt) override;

private:
    int   m_sweepIterations;  ///< 1フレーム内での最大前進反復回数（スライド含む）
    float m_sweepSlop;        ///< 0とみなす許容分離量
    float m_skin;             ///< 接触手前で止めるスキン幅
    float m_groundNormalY;    ///< 接地判定の法線 y 閾値

    struct StaticObbCache {
        WorldObb obb;
    };
    std::vector<StaticObbCache> m_staticObbs;

    void BuildStaticObbCache(class World& world);
};
