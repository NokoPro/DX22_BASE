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

/// @brief 単純なバネ・ダンパ追従（オイラー積分）
/// @param current 現在位置
/// @param velocity 内部速度（呼び出し側で保持してよい）
/// @param target 目標位置
/// @param k バネ係数
/// @param c ダンパ係数
/// @param dt デルタタイム
/// @return 次の位置
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

/// @brief 角度の正規化 [-180,180)
static inline float WrapDeg(float a) { while (a >= 180.f) a -= 360.f; while (a < -180.f) a += 360.f; return a; }

/// @brief C++14互換の clamp
static inline float Clampf(float v, float lo, float hi) { return (v < lo) ? lo : ((v > hi) ? hi : v); }

/**
 * @brief カメラ更新本体。右マウスドラッグ or 矢印キーでオービット回転、PgUp/PgDnでズーム。
 * @details
 * - orbitEnabled=true のとき、ターゲット中心に yaw/pitch で回転し、距離 orbitDistance を保ちます。
 * - 注視点は target.position + lookAtOffset。カメラはそこを常に向きます。
 * - 位置は SpringDamperFollow によりスムーズに追従します。
 */
void FollowCameraSystem::Update(World& world, float dt)
{
    world.View<ActiveCameraTag, CameraRigComponent, TransformComponent>(
        [&](EntityId, ActiveCameraTag&, CameraRigComponent& rig, TransformComponent& camTr)
        {
            if (rig.target == 0) return;
            auto* tgtTr = world.TryGet<TransformComponent>(rig.target);
            if (!tgtTr) return;

            // ---------- 入力（マウス/キー） ----------
            float dYaw = 0.f, dPitch = 0.f, dDist = 0.f;

            // 右ドラッグで回転
            if (IsMouseDownR()) {
                POINT d = GetMouseDelta();
                dYaw -= d.x * rig.sensYaw;
                dPitch -= d.y * rig.sensPitch;
            }

            // キーで水平/垂直回り込み（A/D=水平, W/S=垂直, 矢印も同様）
            auto addYaw = [&](float v) { dYaw += v * rig.sensYaw * 3.0f; };
            auto addPitch = [&](float v) { dPitch += v * rig.sensPitch * 3.0f; };

            // PgUp/PgDn でもズーム（従来）
            if (IsKeyPress(VK_PRIOR)) dDist -= rig.sensZoom;
            if (IsKeyPress(VK_NEXT))  dDist += rig.sensZoom;

            // マウスホイールでズーム（120単位/ノッチ）
            int wheel = ConsumeMouseWheelDelta();
            if (wheel != 0) {
                dDist += -(wheel / 120.0f) * rig.sensZoom; // 前へ回す=近づく
            }

            // ---------- 注視点（支点） ----------
            XMFLOAT3 pivot = {
                tgtTr->position.x + rig.lookAtOffset.x,
                tgtTr->position.y + rig.lookAtOffset.y,
                tgtTr->position.z + rig.lookAtOffset.z
            };

            // ---------- オービット or 追従 ----------
            static XMFLOAT3 vel{ 0,0,0 }; // 追従用の内部速度
            if (rig.orbitEnabled)
            {
                // --- 角度更新・クランプは従来どおり ---
                rig.orbitYawDeg = WrapDeg(rig.orbitYawDeg + dYaw);
                rig.orbitPitchDeg = Clampf(rig.orbitPitchDeg + dPitch, rig.orbitMinPitch, rig.orbitMaxPitch);
                rig.orbitDistance = Clampf(rig.orbitDistance + dDist, rig.orbitMinDist, rig.orbitMaxDist);

                const float yawR = DirectX::XMConvertToRadians(rig.orbitYawDeg);
                const float pitchR = DirectX::XMConvertToRadians(rig.orbitPitchDeg);
                const float cY = std::cos(yawR), sY = std::sin(yawR);
                const float cP = std::cos(pitchR), sP = std::sin(pitchR);

                // 前方・右ベクトル（右はYawのみでOK）
                DirectX::XMFLOAT3 forward{ cP * cY, sP, cP * sY };      // ピボットから見た前
                DirectX::XMFLOAT3 right{ cY,    0.f, sY };     // 水平右（Yaw基準）

                // 1) ブーム：支点から距離ぶん後ろへ
                DirectX::XMFLOAT3 boom{
                    -forward.x * rig.orbitDistance,
                    -forward.y * rig.orbitDistance,
                    -forward.z * rig.orbitDistance
                };

                // 2) フォローオフセットを“オービット座標系（Yaw回転のみ）”で適用
                //    X: 右、Y: 上、Z: 前（ターゲットに対して）として扱う
                DirectX::XMFLOAT3 off = rig.followOffset;
                // Yawで回したZ（前後）とX（左右）を世界へ
                DirectX::XMFLOAT3 offYaw{
                    right.x * off.x + cP * cY * 0.0f + (-sY) * 0.0f, // Xだけ right で十分（行列書くとこうなるが right*x でOK）
                    off.y,                                            // 高さはそのまま
                    right.z * off.x + 0.0f                           // Xだけ right で十分
                };
                // 前後（Z）は「Yawだけ」前方向 = (cY,0,sY) で回す
                offYaw.x += (cY * off.z);
                offYaw.z += (sY * off.z);

                // 3) 最終目標位置
                DirectX::XMFLOAT3 targetPos{
                    pivot.x + boom.x + offYaw.x,
                    pivot.y + boom.y + offYaw.y,
                    pivot.z + boom.z + offYaw.z
                };

                // 4) ばねダンパでスムーズ追従（従来どおり）
                static DirectX::XMFLOAT3 vel{ 0,0,0 };
                camTr.position = SpringDamperFollow(camTr.position, vel, targetPos,
                    rig.posStiffness, rig.posDamping, dt);
                if (rig.lock.lockPosX) camTr.position.x = targetPos.x;
                if (rig.lock.lockPosY) camTr.position.y = targetPos.y;
                if (rig.lock.lockPosZ) camTr.position.z = targetPos.z;

            }
            else
            {
                // 従来のオフセット追従
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

            // ---------- ビュー/プロジェクション ----------
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
