#include "MovementControlSystem.h"
#include "../../World.h"
#include "../../Components/Gameplay/CharacterControllerComponent.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/GroundingComponent.h"
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

using namespace DirectX;

static inline XMFLOAT3 LoadFloat3Add(const XMFLOAT3& a, const XMFLOAT3& b) 
{
    XMFLOAT3 r; XMStoreFloat3(&r, XMLoadFloat3(&a) + XMLoadFloat3(&b)); return r;
}
static inline XMFLOAT3 ScaleFloat3(const XMFLOAT3& v, float s) 
{
    XMFLOAT3 r; XMStoreFloat3(&r, XMLoadFloat3(&v) * s); return r;
}

void MovementControlSystem::Update(World& world, float dt)
{
    if (dt <= 0.f) return;

    world.View<CharacterControllerComponent, RigidbodyComponent, GroundingComponent>(
        [&](EntityId, CharacterControllerComponent& cc, RigidbodyComponent& rb, GroundingComponent& gr)
        {
            if (rb.inverseMass <= 0.f) return; // 静的は無視
            const float mass = 1.0f / rb.inverseMass;

            // --- 目標水平速度（XZ） ---
            // 入力ベクトルを正規化→歩行/走りの最大速度でスケール
            XMVECTOR in = XMVectorSet(cc.moveInput.x, 0.f, cc.moveInput.y, 0.f);
            const float lenSq = XMVectorGetX(XMVector3LengthSq(in));
            if (lenSq > 1e-6f) in = XMVector3Normalize(in);
            const float maxSpeed = cc.runModifier ? m_runSpeed : m_walkSpeed;
            XMVECTOR desiredV = XMVectorScale(in, maxSpeed);

            // 現在の水平速度
            XMVECTOR vNow = XMVectorSet(rb.velocity.x, 0.f, rb.velocity.z, 0.f);

            // 目標に近づくための Δv を dt で実現しようとする加速度
            XMVECTOR dv = desiredV - vNow;
            // a = Δv / dt、ただし大きすぎる場合は最大加速度でクランプ
            XMVECTOR a = XMVectorScale(dv, 1.0f / dt);
            const float aLen = XMVectorGetX(XMVector3Length(a));
            if (aLen > m_maxAccelXZ) {
                a = XMVectorScale(XMVector3Normalize(a), m_maxAccelXZ);
            }

            // Force = mass * a を水平方向に加える
            XMFLOAT3 a3; XMStoreFloat3(&a3, a);
            rb.accumulatedForce = LoadFloat3Add(rb.accumulatedForce, ScaleFloat3(a3, mass));

            // --- ジャンプ（接地時のみ） ---
            if (cc.jumpPressed && gr.grounded) {
                // 上向きに初速を与える（インパルス）
                rb.velocity.y = m_jumpSpeed;
                gr.grounded = false;         // 直後は空中
                gr.timeSinceGrounded = 0.f;  // リセット
            }
        });
}
