#pragma once
#include <DirectXMath.h>
#include "../World.h"
#include "PrefabRegistry.h"

#include "../Components/TransformComponent.h"
#include "../Components/Physics/AabbColliderComponent.h"
#include "../Components/Physics/ObbColliderComponent.h"
#include "../Components/ModelRendererComponent.h"

class Model;

/**
 * @file StaticBoxPrefab.h
 * @brief 静的AABB（床/壁など）を生成するプレハブ
 * @details
 * - SpawnParams.position を AABB の中心位置に、SpawnParams.scale を「ハーフサイズ」として解釈する。
 * - Render は付けない（必要なら別の Render 用プレハブを用意）。
 */
namespace Prefabs {

    /**
     * @brief 静的AABB用のプレハブ関数を作るファクトリ
     * @param model 表示に使うモデル（nullptr 可）
     * @return PrefabRegistry::SpawnFunc（World, SpawnParams -> EntityId）
     * @note SpawnParams の scale を AABB halfExtents として扱います。
     */
    inline PrefabRegistry::SpawnFunc MakeStaticBoxPrefab(Model* model)
    {
        return [model](World& world, const PrefabRegistry::SpawnParams& p) -> EntityId
            {
                const EntityId e = world.Create();

                // 1. TransformComponent (位置と回転を反映)
                auto& tr = world.Add<TransformComponent>(e);
                tr.position = {p.position.x, p.position.y,p.position.z - p.scale.z};
                tr.rotationDeg = p.rotationDeg; // <- 回転を反映
                tr.scale = { p.scale.x,p.scale.y,p.scale.z }; // AABBサイズは scale ではないので見た目用として1固定

                // 2. ModelRendererComponent (モデル描画)
                auto& mr = world.Add<ModelRendererComponent>(e);
                mr.model = model;
                mr.visible = true;

                // 3. ObbColliderComponent (静的衝突)
                auto& col = world.Add<ObbColliderComponent>(e);
                col.isStatic = true;
                col.offset = { 0.f,0.f,p.scale.z };
                col.halfExtents = p.scale;      // ★ AABBの半半径は SpawnParams.scale に依存

                return e;
            };
    }

} // namespace Prefabs
