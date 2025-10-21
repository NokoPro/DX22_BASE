#include "FollowCameraSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Camera/CameraRigComponent.h"
#include "../../Components/Camera/ActiveCameraTag.h"
#include "../../../System/Geometory.h"
#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

/**
 * @brief �΂�-�_���p�i2���n�j�ŃX���[�Y�Ɍ��ݒl��ڕW�ɒǏ]������B
 * @param current ���ݒl
 * @param velocity ���ݑ��x�i�X�V�����j
 * @param target �ڕW�l
 * @param stiffness �΂ˌW��
 * @param damping �_���s���O�W��
 * @param dt �o�ߎ���
 * @return �V�����ʒu
 */
static inline XMFLOAT3 SpringDamperFollow(
    const XMFLOAT3& current, XMFLOAT3& velocity,
    const XMFLOAT3& target, float stiffness, float damping, float dt)
{
    XMVECTOR c = XMLoadFloat3(&current);
    XMVECTOR v = XMLoadFloat3(&velocity);
    XMVECTOR t = XMLoadFloat3(&target);

    XMVECTOR a = stiffness * (t - c) - damping * v;  // �����x = kx + cv
    v = v + a * dt;
    c = c + v * dt;

    XMStoreFloat3(&velocity, v);
    XMFLOAT3 next;
    XMStoreFloat3(&next, c);
    return next;
}

void FollowCameraSystem::Update(World& world, float dt)
{
    using namespace DirectX;

    // �A�N�e�B�u�J������T��
    // �C���O: View<ActiveCameraTag, CameraRigComponent, TransformComponent>( [&](EntityId, CameraRigComponent& rig, TransformComponent& camTr)
    // �C����: View<ActiveCameraTag, CameraRigComponent, TransformComponent>( [&](EntityId, ActiveCameraTag& tag, CameraRigComponent& rig, TransformComponent& camTr)
    world.View<ActiveCameraTag, CameraRigComponent, TransformComponent>(
        [&](EntityId, ActiveCameraTag& /* tag */, CameraRigComponent& rig, TransformComponent& camTr)
        //                    ^^^^^^^^^^^^^^^^^^  <- ActiveCameraTag �̈�����ǉ�
        {
            // �Ǐ]�悪�ݒ肳��Ă��Ȃ���΃X�L�b�v
            if (rig.target == 0) return;

            // �Ǐ]��� Transform ��T��
            TransformComponent* tgtTr = world.TryGet<TransformComponent>(rig.target);
            if (!tgtTr) return;

            // ===== 1) �ڕW�ʒu���Z�o =====
            XMFLOAT3 targetPos = tgtTr->position;
            XMFLOAT3 targetLook = tgtTr->position;

            // �Ǐ]�I�t�Z�b�g�i�^�[�Q�b�g��j
            targetPos.x += rig.followOffset.x;
            targetPos.y += rig.followOffset.y;
            targetPos.z += rig.followOffset.z;

            // �����_�I�t�Z�b�g
            targetLook.x += rig.lookAtOffset.x;
            targetLook.y += rig.lookAtOffset.y;
            targetLook.z += rig.lookAtOffset.z;

            // ===== 2) �����Ǐ]�i�΂˃_���p�j=====
            static XMFLOAT3 vel = { 0, 0, 0 };
            camTr.position = SpringDamperFollow(
                camTr.position, vel, targetPos,
                rig.posStiffness, rig.posDamping, dt);

            // ===== 3) �����b�N =====
            if (rig.lock.lockPosX) camTr.position.x = targetPos.x;
            if (rig.lock.lockPosY) camTr.position.y = targetPos.y;
            if (rig.lock.lockPosZ) camTr.position.z = targetPos.z;

            // ===== 4) ��]�iLookAt�s��j=====
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&targetLook);
            XMVECTOR up = XMVectorSet(0, 1, 0, 0);

            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(rig.fovYDeg),
                rig.aspect, rig.zNear, rig.zFar);

            // ===== 5) �s���ێ� & Geometory�ɓK�p =====
            XMStoreFloat4x4(&m_V, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_P, XMMatrixTranspose(P));
            Geometory::SetView(m_V);
            Geometory::SetProjection(m_P);
        });
}