#pragma once
#include <DirectXMath.h>
#include "../../ECS.h"

/**
 * @file CameraRigComponent.h
 * @brief カメラ挙動（追従／オービット）の調整パラメータを保持するコンポーネント
 * @details
 * - ターゲット（EntityId）を基準に、注視点（lookAtOffset）へカメラを向けます。
 * - orbitEnabled=true のとき、ターゲットを支点に yaw/pitch で回転し、distance を保ちます。
 * - posStiffness/posDamping は追従のバネ・ダンパ係数です（スムーズさの調整）。
 */
struct CameraRigComponent
{
    /** @name ターゲット/画角 */
    ///@{
    EntityId target = 0;                 ///< 追従/注視する対象エンティティ
    float    fovYDeg = 60.f;             ///< 垂直視野角（度）
    float    aspect = 16.f / 9.f;         ///< アスペクト比
    float    zNear = 0.1f;             ///< 近クリップ
    float    zFar = 100.f;            ///< 遠クリップ
    ///@}

    /** @name 追従パラメータ */
    ///@{
    DirectX::XMFLOAT3 followOffset{ 0, 2.0f, -4.0f }; ///< 追従時の相対オフセット
    DirectX::XMFLOAT3 lookAtOffset{ 0, 1.0f,  0.0f }; ///< 注視点オフセット（ターゲット座標系）
    float posStiffness = 28.f;                       ///< 追従バネ係数（大きいほど素早く寄る）
    float posDamping = 10.f;                       ///< 追従ダンパ係数（大きいほど減衰強）
    struct { bool lockPosX = false, lockPosY = false, lockPosZ = false; } lock{}; ///< 軸ロック
    ///@}

    /** @name オービット（ターゲット支点回転） */
    ///@{
    bool  orbitEnabled = true;    ///< true でオービット有効
    float orbitYawDeg = 180.f;  ///< 水平角（度）
    float orbitPitchDeg = 15.f;   ///< 俯仰角（度）
    float orbitMinPitch = -80.f;  ///< 俯仰角の下限
    float orbitMaxPitch = 80.f;  ///< 俯仰角の上限
    float orbitDistance = 4.0f;   ///< ターゲットからの距離
    float orbitMinDist = 1.2f;   ///< 距離の下限
    float orbitMaxDist = 12.0f;  ///< 距離の上限
    ///@}

    /** @name 入力感度 */
    ///@{
    float sensYaw = 0.15f;  ///< Yaw 感度（deg / 入力1.0）
    float sensPitch = 0.12f;  ///< Pitch 感度（deg / 入力1.0）
    float sensZoom = 0.25f;  ///< ズーム感度（m / ステップ）
    ///@}
};
