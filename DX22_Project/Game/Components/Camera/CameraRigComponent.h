#pragma once
#include <DirectXMath.h>
#include <cstdint>

/**
 * @file CameraRigComponent.h
 * @brief カメラの基本プロパティと追従/回転の制御用データ
 * @details
 * - 本コンポーネントを持つ「カメラ用エンティティ」を1つ用意して使う。
 * - 追従先（target）を設定すると、FollowCameraSystem が Transform を更新。
 * - 軸ロックは位置/回転それぞれに対して個別に設定できる。
 */
    struct CameraRigComponent
{
    // ================= 光学系（投影） =================
    float fovYDeg = 60.0f;     ///< 垂直FOV（度）
    float aspect = 16.0f / 9.0f;///< 画面アスペクト
    float zNear = 0.1f;      ///< 近クリップ
    float zFar = 1000.0f;   ///< 遠クリップ

    // ================= 追従ターゲット =================
    /// @note 0 なら無効（EntityId が整数の想定）
    unsigned int target = 0;     ///< 追従先エンティティID（Transform 必須）

    // ワールド座標でのオフセット（ターゲット基準）
    DirectX::XMFLOAT3 followOffset{ 0.0f, 2.0f, -6.0f };
    // ターゲットのどこを見るか（ターゲット位置 + ここ）
    DirectX::XMFLOAT3 lookAtOffset{ 0.0f, 1.0f, 0.0f };

    // ================= ばね-ダンパ係数（平滑化）=================
    float posStiffness = 12.0f;  ///< 位置のばね係数（大きいほど素早く寄る）
    float posDamping = 2.0f;   ///< 位置のダンピング（臨界近辺は ~2√k 目安）
    float rotStiffness = 12.0f;  ///< 回転のばね係数
    float rotDamping = 2.0f;   ///< 回転のダンピング

    // ================= 軸ロック =================
    struct AxisLock {
        bool lockPosX = false, lockPosY = false, lockPosZ = false;
        bool lockRotX = false, lockRotY = false, lockRotZ = false;
    } lock;
};
