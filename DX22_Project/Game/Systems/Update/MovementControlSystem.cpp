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

    auto dot3 = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
        };
    auto sub3 = [](DirectX::XMFLOAT3 a, const DirectX::XMFLOAT3& b) {
        a.x -= b.x; a.y -= b.y; a.z -= b.z; return a;
        };
    auto scale3 = [](const DirectX::XMFLOAT3& v, float s) {
        return DirectX::XMFLOAT3{ v.x * s, v.y * s, v.z * s };
        };
    auto len3 = [](const DirectX::XMFLOAT3& v) {
        return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        };

    world.View<CharacterControllerComponent, RigidbodyComponent, GroundingComponent>(
        [&](EntityId, CharacterControllerComponent& cc, RigidbodyComponent& rb, GroundingComponent& gr)
        {
            if (rb.inverseMass <= 0.f) return; // 静的は無視
            const float mass = 1.0f / rb.inverseMass;

            // --- 入力 → 目標水平速度（XZ） ---
            DirectX::XMVECTOR in = DirectX::XMVectorSet(cc.moveInput.x, 0.f, cc.moveInput.y, 0.f);
            const float lenSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(in));
            if (lenSq > 1e-6f) in = DirectX::XMVector3Normalize(in);

            const float maxSpeed = cc.runModifier ? m_runSpeed : m_walkSpeed;
            DirectX::XMVECTOR desiredVxz = DirectX::XMVectorScale(in, maxSpeed);

            // 現在の水平速度
            DirectX::XMVECTOR vNowxz = DirectX::XMVectorSet(rb.velocity.x, 0.f, rb.velocity.z, 0.f);

            // 目標に近づくための Δv → a = Δv/dt を最大加速度でクランプ
            DirectX::XMVECTOR dv = desiredVxz - vNowxz;
            DirectX::XMVECTOR a = DirectX::XMVectorScale(dv, 1.0f / dt);

            float aLen = DirectX::XMVectorGetX(DirectX::XMVector3Length(a));
            if (aLen > m_maxAccelXZ) {
                a = DirectX::XMVectorScale(DirectX::XMVector3Normalize(a), m_maxAccelXZ);
                aLen = m_maxAccelXZ;
            }

            // --- ★接触面の接線へ投影して「壁法線へ押し付けない」 ---
            // 基本は水平制御だが、壁に当たっているときは法線成分を除去
            if (gr.hasContact) {
                DirectX::XMFLOAT3 n = gr.contactNormal;
                // a(3D) を作る（y成分は 0 のままでOK）
                DirectX::XMFLOAT3 a3;
                DirectX::XMStoreFloat3(&a3, a);

                // a_tangent = a - (a・n) n
                const float aN = dot3(a3, n);
                a3 = sub3(a3, scale3(n, aN));

                // ほぼゼロの時は丸め（振動対策）
                if (len3(a3) < 1e-4f) a3 = { 0,0,0 };

                // 返す
                a = DirectX::XMVectorSet(a3.x, 0.f, a3.z, 0.f);
            }

            // --- Force = mass * a を水平に加える ---
            DirectX::XMFLOAT3 a3; DirectX::XMStoreFloat3(&a3, a);
            rb.accumulatedForce.x += a3.x * mass;
            rb.accumulatedForce.z += a3.z * mass;

            // --- ジャンプ（接地時のみ） ---
            //if (cc.jumpPressed && gr.grounded) {
            //    rb.velocity.y = m_jumpSpeed;     // インパルス
            //    gr.grounded = false;
            //    gr.timeSinceGrounded = 0.f;
            //}

            // --- 微小入力のデッドゾーン（任意：端末ノイズ対策） ---
            if (!cc.runModifier && lenSq < 1e-4f) {
                // 入力がほぼゼロなら、ごく小さい水平速度を丸める
                if (std::fabs(rb.velocity.x) < 1e-3f) rb.velocity.x = 0.f;
                if (std::fabs(rb.velocity.z) < 1e-3f) rb.velocity.z = 0.f;
            }
        });
}
