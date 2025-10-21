#include "PhysicsSystem.h"
#include "../../World.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/MotionDeltaComponent.h"
#include "../../Components/Physics/GroundingComponent.h"
#include <DirectXMath.h>
#include <algorithm>

using namespace DirectX;

void PhysicsSystem::Update(World& world, float dt)
{
    if (dt <= 0.f) return;

    auto dot3 = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
        };
    auto add3 = [](DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3& b) {
        a.x += b.x; a.y += b.y; a.z += b.z; return a;
        };
    auto sub3 = [](DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3& b) {
        a.x -= b.x; a.y -= b.y; a.z -= b.z; return a;
        };
    auto scale3 = [](const DirectX::XMFLOAT3& v, float s) {
        return DirectX::XMFLOAT3{ v.x * s, v.y * s, v.z * s };
        };
    // C++14 互換 clamp 関数
    auto clampf = [](float v, float lo, float hi) {
        return (v < lo) ? lo : (v > hi ? hi : v);
        };

    // m_gravityY はスカラー（例：-9.8f）
    world.View<RigidbodyComponent, MotionDeltaComponent, GroundingComponent>(
        [&](EntityId, RigidbodyComponent& rb, MotionDeltaComponent& md, GroundingComponent& gr)
        {
            if (rb.inverseMass <= 0.f) {
                md.delta = { 0,0,0 };
                return;
            }

            // 重力
            DirectX::XMFLOAT3 g{ 0,0,0 };
            if (rb.gravityEnabled) {
                g = { 0.0f, m_gravityY * rb.gravityScale, 0.0f };
            }

            // ★ 接地中は地面法線方向の重力をカット
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float gN = dot3(g, n);
                if (gN < 0.0f) {
                    g = sub3(g, scale3(n, gN));
                }
            }

            // 加速度 a = g + 外力/m
            DirectX::XMFLOAT3 a = g;
            a = add3(a, scale3(rb.accumulatedForce, rb.inverseMass));

            // ★ 接地中は地面へ押し付ける加速度をカット
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float aN = dot3(a, n);
                if (aN < 0.0f) {
                    a = sub3(a, scale3(n, aN));
                }
            }

            // 半陰的オイラー積分
            rb.velocity.x += a.x * dt;
            rb.velocity.y += a.y * dt;
            rb.velocity.z += a.z * dt;

            // ★ C++14用 clamp に変更
            const float damp = clampf(1.0f - rb.linearDamping, 0.0f, 1.0f);
            rb.velocity.x *= damp;
            rb.velocity.y *= damp;
            rb.velocity.z *= damp;

            // 接地中は地面へ突き刺さらないように
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float vN = dot3(rb.velocity, n);
                if (vN < 0.0f) {
                    rb.velocity.x -= vN * n.x;
                    rb.velocity.y -= vN * n.y;
                    rb.velocity.z -= vN * n.z;
                }
            }

            // MotionDelta に速度積分
            md.delta.x += rb.velocity.x * dt;
            md.delta.y += rb.velocity.y * dt;
            md.delta.z += rb.velocity.z * dt;

            // 外力リセット
            rb.accumulatedForce = { 0,0,0 };
        });
}
