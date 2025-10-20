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
                // �ÓI�F���x/�͖͂����Adelta�̓[����
                md.delta = { 0.f, 0.f, 0.f };
                return;
            }

            // --- �O�́������x ---
            XMVECTOR acc = XMLoadFloat3(&rb.accumulatedForce);        // ���݃t���[���̊O��
            acc = XMVectorScale(acc, rb.inverseMass);                  // a = F * invMass

            // --- �d�́i�����̈ꕔ�j ---
            if (rb.gravityEnabled) {
                acc = acc + XMVectorSet(0.f, m_gravityY * rb.gravityScale, 0.f, 0.f);
            }

            // --- ���x�X�V�iSemi-Implicit Euler�j---
            XMVECTOR v = XMLoadFloat3(&rb.velocity);
            v = v + acc * dt;

            // --- ���x�����i��C��R�I�j---
            // �A�����Ԃ̎w�������ɋ߂��`�Fv *= exp(-damping * dt)
            const float damp = std::max(0.f, rb.linearDamping);
            const float k = (damp > 0.f) ? std::expf(-damp * dt) : 1.f;
            v = v * k;

            XMStoreFloat3(&rb.velocity, v);

            // --- ���t���[���̊�]�ړ��� delta �������o�� ---
            XMVECTOR d = v * dt;
            XMStoreFloat3(&md.delta, d);

            // --- �o�ߎ��Ԃ̍X�V�i�ڒn�L���b�V���j---
            gr.timeSinceGrounded += dt;

            // --- �O�͎͂g���؂����̂ŃN���A ---
            rb.accumulatedForce = { 0.f, 0.f, 0.f };
        });
}
