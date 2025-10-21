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
 * @brief ばね-ダンパ（2次系）でスムーズに現在値を目標に追従させる。
 * @param current 現在値
 * @param velocity 現在速度（更新される）
 * @param target 目標値
 * @param stiffness ばね係数
 * @param damping ダンピング係数
 * @param dt 経過時間
 * @return 新しい位置
 */
static inline XMFLOAT3 SpringDamperFollow(
    const XMFLOAT3& current, XMFLOAT3& velocity,
    const XMFLOAT3& target, float stiffness, float damping, float dt)
{
    XMVECTOR c = XMLoadFloat3(&current);
    XMVECTOR v = XMLoadFloat3(&velocity);
    XMVECTOR t = XMLoadFloat3(&target);

    XMVECTOR a = stiffness * (t - c) - damping * v;  // 加速度 = kx + cv
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

    // アクティブカメラを探す
    // 修正前: View<ActiveCameraTag, CameraRigComponent, TransformComponent>( [&](EntityId, CameraRigComponent& rig, TransformComponent& camTr)
    // 修正後: View<ActiveCameraTag, CameraRigComponent, TransformComponent>( [&](EntityId, ActiveCameraTag& tag, CameraRigComponent& rig, TransformComponent& camTr)
    world.View<ActiveCameraTag, CameraRigComponent, TransformComponent>(
        [&](EntityId, ActiveCameraTag& /* tag */, CameraRigComponent& rig, TransformComponent& camTr)
        //                    ^^^^^^^^^^^^^^^^^^  <- ActiveCameraTag の引数を追加
        {
            // 追従先が設定されていなければスキップ
            if (rig.target == 0) return;

            // 追従先の Transform を探す
            TransformComponent* tgtTr = world.TryGet<TransformComponent>(rig.target);
            if (!tgtTr) return;

            // ===== 1) 目標位置を算出 =====
            XMFLOAT3 targetPos = tgtTr->position;
            XMFLOAT3 targetLook = tgtTr->position;

            // 追従オフセット（ターゲット基準）
            targetPos.x += rig.followOffset.x;
            targetPos.y += rig.followOffset.y;
            targetPos.z += rig.followOffset.z;

            // 注視点オフセット
            targetLook.x += rig.lookAtOffset.x;
            targetLook.y += rig.lookAtOffset.y;
            targetLook.z += rig.lookAtOffset.z;

            // ===== 2) 平滑追従（ばねダンパ）=====
            static XMFLOAT3 vel = { 0, 0, 0 };
            camTr.position = SpringDamperFollow(
                camTr.position, vel, targetPos,
                rig.posStiffness, rig.posDamping, dt);

            // ===== 3) 軸ロック =====
            if (rig.lock.lockPosX) camTr.position.x = targetPos.x;
            if (rig.lock.lockPosY) camTr.position.y = targetPos.y;
            if (rig.lock.lockPosZ) camTr.position.z = targetPos.z;

            // ===== 4) 回転（LookAt行列）=====
            XMVECTOR eye = XMLoadFloat3(&camTr.position);
            XMVECTOR at = XMLoadFloat3(&targetLook);
            XMVECTOR up = XMVectorSet(0, 1, 0, 0);

            XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
            XMMATRIX P = XMMatrixPerspectiveFovLH(
                XMConvertToRadians(rig.fovYDeg),
                rig.aspect, rig.zNear, rig.zFar);

            // ===== 5) 行列を保持 & Geometoryに適用 =====
            XMStoreFloat4x4(&m_V, XMMatrixTranspose(V));
            XMStoreFloat4x4(&m_P, XMMatrixTranspose(P));
            Geometory::SetView(m_V);
            Geometory::SetProjection(m_P);
        });
}