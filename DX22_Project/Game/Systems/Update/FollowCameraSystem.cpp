#include "FollowCameraSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Camera/CameraRigComponent.h"
#include "../../Components/Camera/ActiveCameraTag.h"
#include "../../../System/Geometory.h"
#include "../../../System/Input.h"
#include <DirectXMath.h>
#include <cmath>

using namespace DirectX;

/// @brief �P���ȃo�l�E�_���p�Ǐ]�i�I�C���[�ϕ��j
/// @param current ���݈ʒu
/// @param velocity �������x�i�Ăяo�����ŕێ����Ă悢�j
/// @param target �ڕW�ʒu
/// @param k �o�l�W��
/// @param c �_���p�W��
/// @param dt �f���^�^�C��
/// @return ���̈ʒu
static inline XMFLOAT3 SpringDamperFollow(
    const XMFLOAT3& current, XMFLOAT3& velocity,
    const XMFLOAT3& target, float k, float c, float dt)
{
    XMVECTOR cPos = XMLoadFloat3(&current);
    XMVECTOR vel = XMLoadFloat3(&velocity);
    XMVECTOR tgt = XMLoadFloat3(&target);
    XMVECTOR acc = k * (tgt - cPos) - c * vel;
    vel = vel + acc * dt;
    cPos = cPos + vel * dt;
    XMStoreFloat3(&velocity, vel);
    XMFLOAT3 next; XMStoreFloat3(&next, cPos);
    return next;
}

/// @brief �p�x�̐��K�� [-180,180)
static inline float WrapDeg(float a) { while (a >= 180.f) a -= 360.f; while (a < -180.f) a += 360.f; return a; }

/// @brief C++14�݊��� clamp
static inline float Clampf(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }

/**
 * @brief �J�����X�V�{�́B�E�}�E�X�h���b�O or ���L�[�ŃI�[�r�b�g��]�APgUp/PgDn�ŃY�[���B
 * @details
 * - orbitEnabled=true �̂Ƃ��A�^�[�Q�b�g���S�� yaw/pitch �ŉ�]���A���� orbitDistance ��ۂ��܂��B
 * - �����_�� target.position + lookAtOffset�B�J�����͂�������Ɍ����܂��B
 * - �ʒu�� SpringDamperFollow �ɂ��X���[�Y�ɒǏ]���܂��B
 */
void FollowCameraSystem::Update(World& world, float dt)
{
    world.View<ActiveCameraTag, CameraRigComponent, TransformComponent>(
        [&](EntityId, ActiveCameraTag&, CameraRigComponent& rig, TransformComponent& camTr)
        {
            if (rig.target == 0) return;
            auto* tgtTr = world.TryGet<TransformComponent>(rig.target);
            if (!tgtTr) return;

            // ---------- ���́i�}�E�X/�L�[�j ----------
            float dYaw = 0.f, dPitch = 0.f, dDist = 0.f;

            // �E�h���b�O�ŉ�]
            if (IsMouseDownR()) {
                POINT d = GetMouseDelta();
                dYaw -= d.x * rig.sensYaw;
                dPitch -= d.y * rig.sensPitch;
            }

            // �L�[�Ő���/������荞�݁iA/D=����, W/S=����, �������l�j
            auto addYaw = [&](float v) { dYaw += v * rig.sensYaw * 3.0f; };
            auto addPitch = [&](float v) { dPitch += v * rig.sensPitch * 3.0f; };

            // PgUp/PgDn �ł��Y�[���i�]���j
            if (IsKeyPress(VK_PRIOR)) dDist -= rig.sensZoom;
            if (IsKeyPress(VK_NEXT))  dDist += rig.sensZoom;

            // �}�E�X�z�C�[���ŃY�[���i120�P��/�m�b�`�j
            int wheel = ConsumeMouseWheelDelta();
            if (wheel != 0) {
                dDist += -(wheel / 120.0f) * rig.sensZoom; // �O�։�=�߂Â�
            }

            // ---------- �����_�i�x�_�j ----------
            XMFLOAT3 pivot = {
                tgtTr->position.x + rig.lookAtOffset.x,
                tgtTr->position.y + rig.lookAtOffset.y,
                tgtTr->position.z + rig.lookAtOffset.z
            };

            // ---------- �I�[�r�b�g or �Ǐ] ----------
            static XMFLOAT3 vel{ 0,0,0 }; // �Ǐ]�p�̓������x
            if (rig.orbitEnabled)
            {
                // --- �p�x�X�V�E�N�����v�͏]���ǂ��� ---
                rig.orbitYawDeg = WrapDeg(rig.orbitYawDeg + dYaw);
                rig.orbitPitchDeg = Clampf(rig.orbitPitchDeg + dPitch, rig.orbitMinPitch, rig.orbitMaxPitch);
                rig.orbitDistance = Clampf(rig.orbitDistance + dDist, rig.orbitMinDist, rig.orbitMaxDist);

                const float yawR = DirectX::XMConvertToRadians(rig.orbitYawDeg);
                const float pitchR = DirectX::XMConvertToRadians(rig.orbitPitchDeg);
                const float cY = std::cos(yawR), sY = std::sin(yawR);
                const float cP = std::cos(pitchR), sP = std::sin(pitchR);

                // �O���E�E�x�N�g���i�E��Yaw�݂̂�OK�j
                DirectX::XMFLOAT3 forward{ cP * cY, sP, cP * sY };      // �s�{�b�g���猩���O
                DirectX::XMFLOAT3 right{ cY,    0.f, sY };     // �����E�iYaw��j

                // 1) �u�[���F�x�_���狗���Ԃ����
                DirectX::XMFLOAT3 boom{
                    -forward.x * rig.orbitDistance,
                    -forward.y * rig.orbitDistance,
                    -forward.z * rig.orbitDistance
                };

                // 2) �t�H���[�I�t�Z�b�g���g�I�[�r�b�g���W�n�iYaw��]�̂݁j�h�œK�p
                //    X: �E�AY: ��AZ: �O�i�^�[�Q�b�g�ɑ΂��āj�Ƃ��Ĉ���
                DirectX::XMFLOAT3 off = rig.followOffset;
                // Yaw�ŉ񂵂�Z�i�O��j��X�i���E�j�𐢊E��
                DirectX::XMFLOAT3 offYaw{
                    right.x * off.x + cP * cY * 0.0f + (-sY) * 0.0f, // X���� right �ŏ\���i�s�񏑂��Ƃ����Ȃ邪 right*x ��OK�j
                    off.y,                                            // �����͂��̂܂�
                    right.z * off.x + 0.0f                           // X���� right �ŏ\��
                };
                // �O��iZ�j�́uYaw�����v�O���� = (cY,0,sY) �ŉ�
                offYaw.x += (cY * off.z);
                offYaw.z += (sY * off.z);

                // 3) �ŏI�ڕW�ʒu
                DirectX::XMFLOAT3 targetPos{
                    pivot.x + boom.x + offYaw.x,
                    pivot.y + boom.y + offYaw.y,
                    pivot.z + boom.z + offYaw.z
                };

                // 4) �΂˃_���p�ŃX���[�Y�Ǐ]�i�]���ǂ���j
                static DirectX::XMFLOAT3 vel{ 0,0,0 };
                camTr.position = SpringDamperFollow(camTr.position, vel, targetPos,
                    rig.posStiffness, rig.posDamping, dt);
                if (rig.lock.lockPosX) camTr.position.x = targetPos.x;
                if (rig.lock.lockPosY) camTr.position.y = targetPos.y;
                if (rig.lock.lockPosZ) camTr.position.z = targetPos.z;

            }
            else
            {
                // �]���̃I�t�Z�b�g�Ǐ]
                XMFLOAT3 targetPos = {
                    tgtTr->position.x + rig.followOffset.x,
                    tgtTr->position.y + rig.followOffset.y,
                    tgtTr->position.z + rig.followOffset.z
                };
                camTr.position = SpringDamperFollow(camTr.position, vel, targetPos,
                    rig.posStiffness, rig.posDamping, dt);

                if (rig.lock.lockPosX) camTr.position.x = targetPos.x;
                if (rig.lock.lockPosY) camTr.position.y = targetPos.y;
                if (rig.lock.lockPosZ) camTr.position.z = targetPos.z;
            }

            // ---------- �r���[/�v���W�F�N�V���� ----------
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&pivot);
            XMVECTOR up = XMVectorSet(0, 1, 0, 0);

            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(rig.fovYDeg), rig.aspect, rig.zNear, rig.zFar);

            XMStoreFloat4x4(&m_V, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_P, XMMatrixTranspose(P));
            Geometory::SetView(m_V);
            Geometory::SetProjection(m_P);
        });
}
