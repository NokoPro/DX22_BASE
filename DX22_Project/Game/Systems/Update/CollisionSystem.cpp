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
// �ÓIAABB�����W�i���t���[���F����������΋�ԕ����Ɋg���j
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
    (void)dt; // ����͎g�p���Ȃ�
    BuildStaticAabbCache(world);

    // ���I�FTransform + Rigidbody + MotionDelta + AabbCollider + Grounding
    world.View<TransformComponent, RigidbodyComponent, MotionDeltaComponent, AabbColliderComponent, GroundingComponent>(
        [&](EntityId, TransformComponent& tr, RigidbodyComponent& rb, MotionDeltaComponent& md, const AabbColliderComponent& col, GroundingComponent& gr)
        {
            if (col.isStatic || rb.inverseMass <= 0.f) {
                // �ÓI�̓X�L�b�v�i�{�V�X�e���̑ΏۊO�j
                md.delta = { 0,0,0 };
                return;
            }

            // ���t���[���̐ڒn�t���O�������i�ێ��� timeSinceGrounded �ŊǗ��j
            gr.grounded = false;

            // ���݂̓��IAABB�it=0�j
            Aabb dyn0 = MakeWorldAabb(tr, col);

            XMFLOAT3 delta = md.delta;
            // �����I�ɉ����i�K�i/�ǂ̕����ɑΉ��j
            for (int iter = 0; iter < m_maxIterations; ++iter)
            {
                // 1) �ŏ��ɓ�����ÓIAABB��T���i�ŏ�toi�j
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

                // 2) �X�C�[�v�ł̏Փ˂��Ȃ���� ��C�Ɉړ����ďI��
                if (hitIndex < 0) {
                    tr.position.x += delta.x;
                    tr.position.y += delta.y;
                    tr.position.z += delta.z;
                    delta = { 0,0,0 };
                    break;
                }

                // 3) �Փˎ����܂Ői�߂�i������O�Ŏ~�߂�F�����덷����j
                const float eps = 1e-4f;
                const float travel = std::max(0.0f, minToi - eps);
                tr.position.x += delta.x * travel;
                tr.position.y += delta.y * travel;
                tr.position.z += delta.z * travel;

                // 4) �ڐG�ʂɉ����� �����o������������i�@���ɉ����Ďc�ړ��������j
                //    �@���̐�������[���̎��́A���̎������̎c��ړ��Ƒ��x��ł�����
                if (hitN.x != 0.f) { delta.x = 0.f; rb.velocity.x = 0.f; }
                if (hitN.y != 0.f) { delta.y = 0.f; rb.velocity.y = 0.f; }
                if (hitN.z != 0.f) { delta.z = 0.f; rb.velocity.z = 0.f; }

                // 5) �ڒn�X�V�i��ʂƂ݂Ȃ���@�����H�j
                if (hitN.y > m_groundNormalY) {
                    gr.grounded = true;
                    gr.timeSinceGrounded = 0.f;
                    gr.groundNormal = { 0,1,0 }; // AABB�n�`�Ȃ̂� (0,1,0) �Œ��OK
                }

                // 6) ���IAABB �� �V�����ʒu�ōX�V���Ď��̔�����
                dyn0 = MakeWorldAabb(tr, col);

                // 7) ������A���łɏd�Ȃ��Ă��܂��Ă���ꍇ�̉����o���i���U�j
                {
                    const Aabb& stat = m_staticAabbs[hitIndex].aabb;
                    const XMFLOAT3 pen = ComputePenetration(dyn0, stat);
                    if (pen.x > 0 && pen.y > 0 && pen.z > 0) {
                        // �ŏ����ŉ����o��
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
                            if (dir > 0.f) { // �������։����o���ꂽ = ��ʂɏ����
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

                // �c�ړ����قږ�����ΏI��
                if (std::abs(delta.x) + std::abs(delta.y) + std::abs(delta.z) < 1e-6f)
                    break;
            }

            // ������A������̎c�肪����Έړ��i�ʏ�� 0�j
            tr.position.x += delta.x;
            tr.position.y += delta.y;
            tr.position.z += delta.z;

            // MotionDelta �͏���ς�
            md.delta = { 0,0,0 };
        });
}
