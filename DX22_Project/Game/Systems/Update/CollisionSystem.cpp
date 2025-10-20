#include "CollisionSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/AabbColliderComponent.h"
#include "../../Components/Physics/MotionDeltaComponent.h"
#include "../../Components/Physics/GroundingComponent.h"
#include "../Physics/AabbUtil.h"

using namespace DirectX;

// ----------------------------------------------------------
// 静的AABBを収集（毎フレーム：数が多ければ空間分割に拡張）
// ----------------------------------------------------------
void CollisionSystem::BuildStaticAabbCache(World& world)
{
    m_staticAabbs.clear();
    world.View<TransformComponent, AabbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const AabbColliderComponent& col)
        {
            if (!col.isStatic) return;
            m_staticAabbs.push_back({ MakeWorldAabb(tr, col) });
        });
}

// ----------------------------------------------------------
// Update
// ----------------------------------------------------------
void CollisionSystem::Update(World& world, float dt)
{
    (void)dt; // 現状は使用しない
    BuildStaticAabbCache(world);

    // 動的：Transform + Rigidbody + MotionDelta + AabbCollider + Grounding
    world.View<TransformComponent, RigidbodyComponent, MotionDeltaComponent, AabbColliderComponent, GroundingComponent>(
        [&](EntityId, TransformComponent& tr, RigidbodyComponent& rb, MotionDeltaComponent& md, const AabbColliderComponent& col, GroundingComponent& gr)
        {
            if (col.isStatic || rb.inverseMass <= 0.f) {
                // 静的はスキップ（本システムの対象外）
                md.delta = { 0,0,0 };
                return;
            }

            // 毎フレームの接地フラグ初期化（保持は timeSinceGrounded で管理）
            gr.grounded = false;

            // 現在の動的AABB（t=0）
            Aabb dyn0 = MakeWorldAabb(tr, col);

            XMFLOAT3 delta = md.delta;
            // 反復的に解決（階段/壁の複合に対応）
            for (int iter = 0; iter < m_maxIterations; ++iter)
            {
                // 1) 最初に当たる静的AABBを探索（最小toi）
                float minToi = 1.0f;
                XMFLOAT3 hitN{ 0,0,0 };
                int hitIndex = -1;

                for (int i = 0; i < (int)m_staticAabbs.size(); ++i)
                {
                    const SweptHit h = SweptAabb(dyn0, delta, m_staticAabbs[i].aabb);
                    if (h.hit && h.toi < minToi) {
                        minToi = h.toi;
                        hitN = h.normal;
                        hitIndex = i;
                    }
                }

                // 2) スイープでの衝突がなければ 一気に移動して終了
                if (hitIndex < 0) {
                    tr.position.x += delta.x;
                    tr.position.y += delta.y;
                    tr.position.z += delta.z;
                    delta = { 0,0,0 };
                    break;
                }

                // 3) 衝突時刻まで進める（少し手前で止める：浮動誤差回避）
                const float eps = 1e-4f;
                const float travel = std::max(0.0f, minToi - eps);
                tr.position.x += delta.x * travel;
                tr.position.y += delta.y * travel;
                tr.position.z += delta.z * travel;

                // 4) 接触面に沿って 押し出し方向を決定（法線に沿って残移動を消す）
                //    法線の成分が非ゼロの軸は、その軸成分の残り移動と速度を打ち消す
                if (hitN.x != 0.f) { delta.x = 0.f; rb.velocity.x = 0.f; }
                if (hitN.y != 0.f) { delta.y = 0.f; rb.velocity.y = 0.f; }
                if (hitN.z != 0.f) { delta.z = 0.f; rb.velocity.z = 0.f; }

                // 5) 接地更新（上面とみなせる法線か？）
                if (hitN.y > m_groundNormalY) {
                    gr.grounded = true;
                    gr.timeSinceGrounded = 0.f;
                    gr.groundNormal = { 0,1,0 }; // AABB地形なので (0,1,0) 固定でOK
                }

                // 6) 動的AABB を 新しい位置で更新して次の反復へ
                dyn0 = MakeWorldAabb(tr, col);

                // 7) 万が一、すでに重なってしまっている場合の押し出し（離散）
                {
                    const Aabb& stat = m_staticAabbs[hitIndex].aabb;
                    const XMFLOAT3 pen = ComputePenetration(dyn0, stat);
                    if (pen.x > 0 && pen.y > 0 && pen.z > 0) {
                        // 最小軸で押し出す
                        if (pen.x <= pen.y && pen.x <= pen.z) {
                            const float dir = (dyn0.min.x + dyn0.max.x < stat.min.x + stat.max.x) ? -1.f : +1.f;
                            tr.position.x += dir * pen.x;
                            rb.velocity.x = 0.f;
                            delta.x = 0.f;
                        }
                        else if (pen.y <= pen.x && pen.y <= pen.z) {
                            const float dir = (dyn0.min.y + dyn0.max.y < stat.min.y + stat.max.y) ? -1.f : +1.f;
                            tr.position.y += dir * pen.y;
                            rb.velocity.y = 0.f;
                            delta.y = 0.f;
                            if (dir > 0.f) { // 下から上へ押し出された = 上面に乗った
                                gr.grounded = true;
                                gr.timeSinceGrounded = 0.f;
                                gr.groundNormal = { 0,1,0 };
                            }
                        }
                        else {
                            const float dir = (dyn0.min.z + dyn0.max.z < stat.min.z + stat.max.z) ? -1.f : +1.f;
                            tr.position.z += dir * pen.z;
                            rb.velocity.z = 0.f;
                            delta.z = 0.f;
                        }
                        dyn0 = MakeWorldAabb(tr, col);
                    }
                }

                // 残移動がほぼ無ければ終了
                if (std::abs(delta.x) + std::abs(delta.y) + std::abs(delta.z) < 1e-6f)
                    break;
            }

            // 反復後、未消費の残りがあれば移動（通常は 0）
            tr.position.x += delta.x;
            tr.position.y += delta.y;
            tr.position.z += delta.z;

            // MotionDelta は消費済み
            md.delta = { 0,0,0 };
        });
}
