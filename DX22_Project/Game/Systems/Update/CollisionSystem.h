#pragma once
#include "../IUpdateSystem.h"
#include <vector>
#include <DirectXMath.h>
#include "../Physics/AabbUtil.h"

/**
 * @file CollisionSystem.h
 * @brief MotionDelta を静的AABBに対して解決し、Transform を確定・接地更新するSystem
 * @details
 * - 入力：Transform, Rigidbody, MotionDelta, AabbCollider（動的）／ Transform, AabbCollider（静的）
 * - 手順：
 *   1) 静的AABBを事前収集
 *   2) 各動的に対して「スイープ（SweptAabb）→ 押し出し（重なり）→ 速度修正 → 接地更新」
 *   3) 最終 Transform.position を決定（delta を消費）
 * - 連続衝突に備え、最大反復回数を設ける（デフォ 3）。
 */
class CollisionSystem final : public IUpdateSystem {
public:
    explicit CollisionSystem(int maxIterations = 3, float groundNormalY = 0.6f)
        : m_maxIterations(maxIterations), m_groundNormalY(groundNormalY) {
    }

    void Update(class World& world, float dt) override;

private:
    int   m_maxIterations; ///< 1フレーム内の最大反復数
    float m_groundNormalY; ///< これ以上なら「上面」とみなして接地扱い

    struct StaticAabbCache 
    {
        Aabb aabb;
    };

    std::vector<StaticAabbCache> m_staticAabbs;

    void BuildStaticAabbCache(class World& world);
};
