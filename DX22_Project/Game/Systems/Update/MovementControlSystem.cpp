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
            if (rb.inverseMass <= 0.f) return; // �ÓI�͖���
            const float mass = 1.0f / rb.inverseMass;

            // --- ���� �� �ڕW�������x�iXZ�j ---
            DirectX::XMVECTOR in = DirectX::XMVectorSet(cc.moveInput.x, 0.f, cc.moveInput.y, 0.f);
            const float lenSq = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(in));
            if (lenSq > 1e-6f) in = DirectX::XMVector3Normalize(in);

            const float maxSpeed = cc.runModifier ? m_runSpeed : m_walkSpeed;
            DirectX::XMVECTOR desiredVxz = DirectX::XMVectorScale(in, maxSpeed);

            // ���݂̐������x
            DirectX::XMVECTOR vNowxz = DirectX::XMVectorSet(rb.velocity.x, 0.f, rb.velocity.z, 0.f);

            // �ڕW�ɋ߂Â����߂� ��v �� a = ��v/dt ���ő�����x�ŃN�����v
            DirectX::XMVECTOR dv = desiredVxz - vNowxz;
            DirectX::XMVECTOR a = DirectX::XMVectorScale(dv, 1.0f / dt);

            float aLen = DirectX::XMVectorGetX(DirectX::XMVector3Length(a));
            if (aLen > m_maxAccelXZ) {
                a = DirectX::XMVectorScale(DirectX::XMVector3Normalize(a), m_maxAccelXZ);
                aLen = m_maxAccelXZ;
            }

            // --- ���ڐG�ʂ̐ڐ��֓��e���āu�ǖ@���։����t���Ȃ��v ---
            // ��{�͐������䂾���A�ǂɓ������Ă���Ƃ��͖@������������
            if (gr.hasContact) {
                DirectX::XMFLOAT3 n = gr.contactNormal;
                // a(3D) �����iy������ 0 �̂܂܂�OK�j
                DirectX::XMFLOAT3 a3;
                DirectX::XMStoreFloat3(&a3, a);

                // a_tangent = a - (a�En) n
                const float aN = dot3(a3, n);
                a3 = sub3(a3, scale3(n, aN));

                // �قڃ[���̎��͊ۂ߁i�U���΍�j
                if (len3(a3) < 1e-4f) a3 = { 0,0,0 };

                // �Ԃ�
                a = DirectX::XMVectorSet(a3.x, 0.f, a3.z, 0.f);
            }

            // --- Force = mass * a �𐅕��ɉ����� ---
            DirectX::XMFLOAT3 a3; DirectX::XMStoreFloat3(&a3, a);
            rb.accumulatedForce.x += a3.x * mass;
            rb.accumulatedForce.z += a3.z * mass;

            // --- �W�����v�i�ڒn���̂݁j ---
            //if (cc.jumpPressed && gr.grounded) {
            //    rb.velocity.y = m_jumpSpeed;     // �C���p���X
            //    gr.grounded = false;
            //    gr.timeSinceGrounded = 0.f;
            //}

            // --- �������͂̃f�b�h�]�[���i�C�ӁF�[���m�C�Y�΍�j ---
            if (!cc.runModifier && lenSq < 1e-4f) {
                // ���͂��قڃ[���Ȃ�A�����������������x���ۂ߂�
                if (std::fabs(rb.velocity.x) < 1e-3f) rb.velocity.x = 0.f;
                if (std::fabs(rb.velocity.z) < 1e-3f) rb.velocity.z = 0.f;
            }
        });
}
