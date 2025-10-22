#include "CollisionSystemOBB.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/ObbColliderComponent.h"
#include "../../Components/Physics/MotionDeltaComponent.h"
#include "../../Components/Physics/GroundingComponent.h"

using namespace DirectX;

// ----------------------------------------------------------
// �ÓIOBB�����W
// ----------------------------------------------------------
void CollisionSystemOBB::BuildStaticObbCache(World& world)
{
    m_staticObbs.clear();
    world.View<TransformComponent, ObbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const ObbColliderComponent& col)
        {
            if (!col.isStatic) return;
            m_staticObbs.push_back({ MakeWorldObb(tr, col) });
        });
}

// �����[�e�B���e�B
static inline float Dot3(const XMFLOAT3& a, const XMFLOAT3& b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
static inline float Len3(const XMFLOAT3& v) {
    return std::fabs(v.x) + std::fabs(v.y) + std::fabs(v.z);
}

// ----------------------------------------------------------
// Update : �X�C�[�v�i�A������j�ŐڐG��O�Ɏ~�߂�
// ----------------------------------------------------------
void CollisionSystemOBB::Update(World& world, float dt)
{
    BuildStaticObbCache(world);

    // --- �p�����[�^ ---
    const float kEnterGroundY = 0.70f;
    const float kExitGroundY = 0.50f;
    const float kSnapDist = std::max(2.0f * m_skin, 0.004f);
    const float kVelEps = 1e-3f;
    const float kMinRemainEps = 1e-6f;
    const float kTangentDampFloor = 0.02f;
    const float kTangentDampWall = 0.04f;
    const float kWallY = 0.30f;
    const float kGlanceCosMax = 0.173648f; // cos(80��)
    const float kSideSkinScale = 1.5f;
    const float kMinPushOut = 0.0002f;

    auto dot3 = [](const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        return a.x * b.x + a.y * b.y + a.z * b.z;
        };
    auto len2 = [](const DirectX::XMFLOAT3& v) {
        return v.x * v.x + v.y * v.y + v.z * v.z;
        };
    auto normalize3 = [](DirectX::XMFLOAT3 v) {
        const float l = std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
        if (l > 0.0f) { v.x /= l; v.y /= l; v.z /= l; }
        return v;
        };

    world.View<TransformComponent, RigidbodyComponent,
        MotionDeltaComponent, ObbColliderComponent, GroundingComponent>(
            [&](EntityId, TransformComponent& tr, RigidbodyComponent& rb,
                MotionDeltaComponent& md, const ObbColliderComponent& col, GroundingComponent& gr)
            {
                if (col.isStatic || rb.inverseMass <= 0.f) {
                    md.delta = { 0,0,0 };
                    return;
                }

                // === �����Ŗ��t���[�����Z�b�g ===
                gr.hasContact = false;
                gr.contactNormal = { 0,0,0 };

                gr.timeSinceGrounded += dt;

                // --- �f�y�l�g���[�V�����i�ŏ��ɏd�Ȃ��Ă����ꍇ�j ---
                {
                    const WorldObb dyn0 = MakeWorldObb(tr, col);
                    bool overlapped = false;
                    float bestDepth = 0.0f;
                    DirectX::XMFLOAT3 bestN{ 0,0,0 };

                    for (const auto& s : m_staticObbs) {
                        const ObbHit hit = IntersectOBB(dyn0, s.obb);
                        if (!hit.hit) continue;
                        if (!overlapped || hit.depth > bestDepth) {
                            overlapped = true; bestDepth = hit.depth; bestN = hit.normal;
                        }
                    }
                    if (overlapped) {
                        tr.position.x += bestN.x * (bestDepth + m_skin);
                        tr.position.y += bestN.y * (bestDepth + m_skin);
                        tr.position.z += bestN.z * (bestDepth + m_skin);

                        // �� �d�Ȃ莞���ڐG����
                        gr.hasContact = true;
                        gr.contactNormal = bestN;
                    }
                }

                DirectX::XMFLOAT3 remain = md.delta;
                bool hitGroundThisFrame = false;
                DirectX::XMFLOAT3 lastHitN{ 0,0,0 };
                bool haveSmoothN = false;
                DirectX::XMFLOAT3 smoothN{ 0,0,0 };

                for (int iter = 0; iter < m_sweepIterations; ++iter)
                {
                    if (len2(remain) < kMinRemainEps * kMinRemainEps) break;

                    const WorldObb dyn = MakeWorldObb(tr, col);
                    float bestT = 1.0f; int bestIdx = -1; RayHit bestHit{};

                    for (int i = 0; i < (int)m_staticObbs.size(); ++i) {
                        const RayHit h = RaycastPointVsExpandedObb(
                            m_staticObbs[i].obb, dyn, dyn.center, remain, m_skin);
                        if (!h.hit) continue;
                        if (dot3(remain, h.normal) >= 0.0f) continue;
                        if (h.t < bestT) { bestT = h.t; bestIdx = i; bestHit = h; }
                    }

                    if (bestIdx < 0) {
                        // �q�b�g�Ȃ����S�ʈړ�
                        tr.position.x += remain.x;
                        tr.position.y += remain.y;
                        tr.position.z += remain.z;
                        remain = { 0,0,0 };
                        break;
                    }

                    // t���i�߂ĉ����o��
                    tr.position.x += remain.x * bestT;
                    tr.position.y += remain.y * bestT;
                    tr.position.z += remain.z * bestT;

                    DirectX::XMFLOAT3 n = bestHit.normal;

                    // --- �� contactNormal �̍X�V�������� ---
                    gr.hasContact = true;
                    gr.contactNormal = n;

                    // �@���X���[�W���O
                    if (haveSmoothN) {
                        const float cosAng = dot3(smoothN, n);
                        if (cosAng > 0.866025f)
                            n = normalize3({ smoothN.x + n.x, smoothN.y + n.y, smoothN.z + n.z });
                        else smoothN = n;
                    }
                    else {
                        smoothN = n; haveSmoothN = true;
                    }

                    // �i���p�ɂ�鉟���o������
                    const float remLen = std::sqrt(len2(remain));
                    const float cosInc = std::fabs(dot3(remain, n)) / (remLen + 1e-9f);
                    const bool isWall = (std::fabs(n.y) < kWallY);
                    const float skinBase = isWall ? (m_skin * kSideSkinScale) : m_skin;
                    const float pushScale = (cosInc < kGlanceCosMax)
                        ? 0.30f
                        : (0.30f + 0.70f * ((cosInc - kGlanceCosMax) / (1.0f - kGlanceCosMax)));
                    const float pushOut = std::max(skinBase * pushScale, kMinPushOut);

                    // �O���։����o��
                    tr.position.x += n.x * pushOut;
                    tr.position.y += n.y * pushOut;
                    tr.position.z += n.z * pushOut;

                    // �@�������̑��x������
                    const float vN = dot3(rb.velocity, n);
                    if (vN < 0.0f) {
                        rb.velocity.x -= vN * n.x;
                        rb.velocity.y -= vN * n.y;
                        rb.velocity.z -= vN * n.z;
                    }

                    // ��/���Őڐ�����
                    {
                        const float vN2 = dot3(rb.velocity, n);
                        DirectX::XMFLOAT3 vt{ rb.velocity.x - vN2 * n.x,
                                             rb.velocity.y - vN2 * n.y,
                                             rb.velocity.z - vN2 * n.z };
                        const float damp = isWall ? kTangentDampWall : kTangentDampFloor;
                        rb.velocity.x -= vt.x * damp;
                        rb.velocity.y -= vt.y * damp;
                        rb.velocity.z -= vt.z * damp;
                        if (std::fabs(rb.velocity.x) < kVelEps) rb.velocity.x = 0;
                        if (std::fabs(rb.velocity.y) < kVelEps) rb.velocity.y = 0;
                        if (std::fabs(rb.velocity.z) < kVelEps) rb.velocity.z = 0;
                    }

                    // �c����X���C�h
                    const float rN = dot3(remain, n);
                    remain.x -= rN * n.x;
                    remain.y -= rN * n.y;
                    remain.z -= rN * n.z;

                    if (!isWall && n.y > kEnterGroundY) {
                        hitGroundThisFrame = true;
                        lastHitN = n;
                        if (rb.velocity.y < 0.0f) rb.velocity.y = 0.0f;
                    }

                    if (len2(remain) < (m_sweepSlop * m_sweepSlop)) {
                        remain = { 0,0,0 };
                        break;
                    }
                }

                // --- (2) �n�ʃX�i�b�v�F�����ȕ���/���݂��z�� ---
                if (!hitGroundThisFrame && rb.velocity.y <= 0.0f && std::fabs(rb.velocity.y) < 0.2f)
                {
                    const WorldObb dyn = MakeWorldObb(tr, col);
                    DirectX::XMFLOAT3 down{ 0,-kSnapDist,0 };
                    float bestT = 1.0f; bool snapped = false; DirectX::XMFLOAT3 n{ 0,0,0 };

                    for (int i = 0; i < (int)m_staticObbs.size(); ++i) 
                    {
                        const RayHit h = RaycastPointVsExpandedObb(m_staticObbs[i].obb, dyn, dyn.center, down, m_skin);
                        if (!h.hit) continue;
                        if (h.normal.y < kEnterGroundY) continue; // ������ʂ̂�
                        if (h.t < bestT) { bestT = h.t; n = h.normal; snapped = true; }
                    }
                    if (snapped) 
                    {
                        tr.position.y += down.y * bestT;
                        tr.position.x += n.x * m_skin;
                        tr.position.y += n.y * m_skin;
                        tr.position.z += n.z * m_skin;

                        // �� �u�������X�i�b�v�v�Ɍ����Đڒn�����ɂ���
                        hitGroundThisFrame = true;
                        lastHitN = n;

                        // �������������x�͊ۂ߂�i������͐G��Ȃ��j
                        if (rb.velocity.y < 0.0f) rb.velocity.y = 0.0f;
                    }
                }

                // --- (3) �ڒn�̃q�X�e���V�X�m�� ---
                if (hitGroundThisFrame)
                {
                    gr.grounded = true;
                    gr.timeSinceGrounded = 0.f;
                    gr.groundNormal = lastHitN;
                    // ����̔��������͊ۂ�
                    if (std::fabs(rb.velocity.y) < kVelEps) rb.velocity.y = 0.f;
                }
                else
                {
                    gr.grounded = false; // �� �������d�v�I
                    // ��ڒn�Ȃ̂ŁAPhysics ���ŏd�͂��t���K�p�ł���
                }


                // �t���[���̈ړ�������
                md.delta = { 0,0,0 };
            });
}
