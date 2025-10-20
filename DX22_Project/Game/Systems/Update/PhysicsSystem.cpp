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

    world.View<RigidbodyComponent, MotionDeltaComponent, GroundingComponent>(
        [&](EntityId, RigidbodyComponent& rb, MotionDeltaComponent& md, GroundingComponent& gr)
        {
            if (rb.inverseMass <= 0.f) {
                // 静的：速度/力は無視、deltaはゼロに
                md.delta = { 0.f, 0.f, 0.f };
                return;
            }

            // --- 外力→加速度 ---
            XMVECTOR acc = XMLoadFloat3(&rb.accumulatedForce);        // 現在フレームの外力
            acc = XMVectorScale(acc, rb.inverseMass);                  // a = F * invMass

            // --- 重力（物理の一部） ---
            if (rb.gravityEnabled) {
                acc = acc + XMVectorSet(0.f, m_gravityY * rb.gravityScale, 0.f, 0.f);
            }

            // --- 速度更新（Semi-Implicit Euler）---
            XMVECTOR v = XMLoadFloat3(&rb.velocity);
            v = v + acc * dt;

            // --- 速度減衰（空気抵抗的）---
            // 連続時間の指数減衰に近い形：v *= exp(-damping * dt)
            const float damp = std::max(0.f, rb.linearDamping);
            const float k = (damp > 0.f) ? std::expf(-damp * dt) : 1.f;
            v = v * k;

            XMStoreFloat3(&rb.velocity, v);

            // --- 今フレームの希望移動量 delta を書き出す ---
            XMVECTOR d = v * dt;
            XMStoreFloat3(&md.delta, d);

            // --- 経過時間の更新（接地キャッシュ）---
            gr.timeSinceGrounded += dt;

            // --- 外力は使い切ったのでクリア ---
            rb.accumulatedForce = { 0.f, 0.f, 0.f };
        });
}
