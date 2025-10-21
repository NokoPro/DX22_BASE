#pragma once
#include <DirectXMath.h>
#include "../World.h"
#include "PrefabRegistry.h"

// === 必要コンポーネント ===
#include "../Components/TransformComponent.h"
#include "../Components/ModelRendererComponent.h"
#include "../Components/Gameplay/CharacterControllerComponent.h"
#include "../Components/Physics/RigidbodyComponent.h"
#include "../Components/Physics/AabbColliderComponent.h"
#include "../Components/Physics/GroundingComponent.h"
#include "../Components/Physics/MotionDeltaComponent.h"
#include "../Components/Physics/ObbColliderComponent.h"

class Model;

/**
 * @file PlayerPrefab.h
 * @brief Player エンティティを生成するプレハブ（生成レシピ）
 * @details
 * - Transform / ModelRenderer / CharacterController / Rigidbody /
 *   AabbCollider / Grounding / MotionDelta を一括付与する。
 * - Model* は外部（Scene など）の資産をキャプチャして渡す。
 */
namespace Prefabs {

    /// プレイヤーの調整用パラメータ
    struct PlayerConfig {
        Model* model = nullptr;                         ///< 表示に使うモデル
        DirectX::XMFLOAT3 colliderHalf{ 0.2f,0.3f,0.2f }; ///< AABBハーフサイズ
        DirectX::XMFLOAT3 colliderOffset{ 0.f,0.3f,0.f }; ///< AABBオフセット
        float inverseMass = 1.0f;                    ///< 1/mass（0で静的）
        float linearDamping = 0.05f;                   ///< 速度減衰
        bool  gravityEnabled = true;                    ///< 重力ON/OFF
        float gravityScale = 1.0f;                    ///< 重力倍率
    };

    /**
     * @brief Player 用プレハブ生成関数を作るファクトリ
     * @param cfg 調整パラメータ（Model* 含む）
     * @return PrefabRegistry::SpawnFunc（World, SpawnParams -> EntityId）
     * @details
     * - SpawnParams.position/rotationDeg/scale を Transform に反映。
     * - scale はモデルの見た目用（AABBサイズは cfg.colliderHalf を使う）。
     */
    inline PrefabRegistry::SpawnFunc MakePlayerPrefab(const PlayerConfig& cfg)
    {
        return [cfg](World& world, const PrefabRegistry::SpawnParams& p) -> EntityId
            {
                const EntityId e = world.Create();

                // Transform
                auto& tr = world.Add<TransformComponent>(e);
                tr.position = p.position;
                tr.rotationDeg = p.rotationDeg;
                tr.scale = p.scale;

                // Render
                auto& mr = world.Add<ModelRendererComponent>(e);
                mr.model = cfg.model;   // null でも生成自体は可能
                mr.visible = true;

                // Input（意図）
                world.Add<CharacterControllerComponent>(e);

                // Rigidbody（物理）
                auto& rb = world.Add<RigidbodyComponent>(e);
                rb.inverseMass = cfg.inverseMass;
                rb.linearDamping = cfg.linearDamping;
                rb.gravityEnabled = cfg.gravityEnabled;
                rb.gravityScale = cfg.gravityScale;

                // OBB（動的）
                auto& col = world.Add<ObbColliderComponent>(e);
                col.isStatic = false;
                col.offset = cfg.colliderOffset;
                col.halfExtents = cfg.colliderHalf;
                col.rotationDeg = { 0.f, 0.f, 0.f };

                // Grounding / MotionDelta
                world.Add<GroundingComponent>(e);
                world.Add<MotionDeltaComponent>(e);

                return e;
            };
    }

} // namespace Prefabs
