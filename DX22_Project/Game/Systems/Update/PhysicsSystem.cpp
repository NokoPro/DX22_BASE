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
    // C++14 �݊� clamp �֐�
    auto clampf = [](float v, float lo, float hi) {
        return (v < lo) ? lo : (v > hi ? hi : v);
        };

    // m_gravityY �̓X�J���[�i��F-9.8f�j
    world.View<RigidbodyComponent, MotionDeltaComponent, GroundingComponent>(
        [&](EntityId, RigidbodyComponent& rb, MotionDeltaComponent& md, GroundingComponent& gr)
        {
            if (rb.inverseMass <= 0.f) {
                md.delta = { 0,0,0 };
                return;
            }

            // �d��
            DirectX::XMFLOAT3 g{ 0,0,0 };
            if (rb.gravityEnabled) {
                g = { 0.0f, m_gravityY * rb.gravityScale, 0.0f };
            }

            // �� �ڒn���͒n�ʖ@�������̏d�͂��J�b�g
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float gN = dot3(g, n);
                if (gN < 0.0f) {
                    g = sub3(g, scale3(n, gN));
                }
            }

            // �����x a = g + �O��/m
            DirectX::XMFLOAT3 a = g;
            a = add3(a, scale3(rb.accumulatedForce, rb.inverseMass));

            // �� �ڒn���͒n�ʂ։����t��������x���J�b�g
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float aN = dot3(a, n);
                if (aN < 0.0f) {
                    a = sub3(a, scale3(n, aN));
                }
            }

            // ���A�I�I�C���[�ϕ�
            rb.velocity.x += a.x * dt;
            rb.velocity.y += a.y * dt;
            rb.velocity.z += a.z * dt;

            // �� C++14�p clamp �ɕύX
            const float damp = clampf(1.0f - rb.linearDamping, 0.0f, 1.0f);
            rb.velocity.x *= damp;
            rb.velocity.y *= damp;
            rb.velocity.z *= damp;

            // �ڒn���͒n�ʂ֓˂��h����Ȃ��悤��
            if (gr.grounded) {
                const DirectX::XMFLOAT3& n = gr.groundNormal;
                const float vN = dot3(rb.velocity, n);
                if (vN < 0.0f) {
                    rb.velocity.x -= vN * n.x;
                    rb.velocity.y -= vN * n.y;
                    rb.velocity.z -= vN * n.z;
                }
            }

            // MotionDelta �ɑ��x�ϕ�
            md.delta.x += rb.velocity.x * dt;
            md.delta.y += rb.velocity.y * dt;
            md.delta.z += rb.velocity.z * dt;

            // �O�̓��Z�b�g
            rb.accumulatedForce = { 0,0,0 };
        });
}
