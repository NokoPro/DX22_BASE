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
            if (rb.inverseMass <= 0.f) return; // �ÓI�͖���
            const float mass = 1.0f / rb.inverseMass;

            // --- �ڕW�������x�iXZ�j ---
            // ���̓x�N�g���𐳋K�������s/����̍ő呬�x�ŃX�P�[��
            XMVECTOR in = XMVectorSet(cc.moveInput.x, 0.f, cc.moveInput.y, 0.f);
            const float lenSq = XMVectorGetX(XMVector3LengthSq(in));
            if (lenSq > 1e-6f) in = XMVector3Normalize(in);
            const float maxSpeed = cc.runModifier ? m_runSpeed : m_walkSpeed;
            XMVECTOR desiredV = XMVectorScale(in, maxSpeed);

            // ���݂̐������x
            XMVECTOR vNow = XMVectorSet(rb.velocity.x, 0.f, rb.velocity.z, 0.f);

            // �ڕW�ɋ߂Â����߂� ��v �� dt �Ŏ������悤�Ƃ�������x
            XMVECTOR dv = desiredV - vNow;
            // a = ��v / dt�A�������傫������ꍇ�͍ő�����x�ŃN�����v
            XMVECTOR a = XMVectorScale(dv, 1.0f / dt);
            const float aLen = XMVectorGetX(XMVector3Length(a));
            if (aLen > m_maxAccelXZ) {
                a = XMVectorScale(XMVector3Normalize(a), m_maxAccelXZ);
            }

            // Force = mass * a �𐅕������ɉ�����
            XMFLOAT3 a3; XMStoreFloat3(&a3, a);
            rb.accumulatedForce = LoadFloat3Add(rb.accumulatedForce, ScaleFloat3(a3, mass));

            // --- �W�����v�i�ڒn���̂݁j ---
            if (cc.jumpPressed && gr.grounded) {
                // ������ɏ�����^����i�C���p���X�j
                rb.velocity.y = m_jumpSpeed;
                gr.grounded = false;         // ����͋�
                gr.timeSinceGrounded = 0.f;  // ���Z�b�g
            }
        });
}
