#pragma once
#include <DirectXMath.h>
#include "../World.h"
#include "PrefabRegistry.h"

#include "../Components/TransformComponent.h"
#include "../Components/Physics/AabbColliderComponent.h"
#include "../Components/Physics/ObbColliderComponent.h"

/**
 * @file StaticBoxPrefab.h
 * @brief 静的AABB（床/壁など）を生成するプレハブ
 * @details
 * - SpawnParams.position を AABB の中心位置に、SpawnParams.scale を「ハーフサイズ」として解釈する。
 * - Render は付けない（必要なら別の Render 用プレハブを用意）。
 */
namespace Prefabs {

    /**
     * @brief 静的AABB用のプレハブ関数を返す（設定は特に不要）
     * @return PrefabRegistry::SpawnFunc（World, SpawnParams -> EntityId）
     * @note SpawnParams の scale を AABB halfExtents として扱います。
     */
    inline PrefabRegistry::SpawnFunc MakeStaticBoxPrefab()
    {
        return [](World& world, const PrefabRegistry::SpawnParams& p) -> EntityId
            {
                const EntityId e = world.Create();

                auto& tr = world.Add<TransformComponent>(e);
                tr.position = p.position;
                tr.rotationDeg = p.rotationDeg; // AABBは軸平行前提なので通常は(0,0,0)
                tr.scale = { 1.f,1.f,1.f }; // 見た目用ではないので1固定

                auto& col = world.Add<ObbColliderComponent>(e);
                col.isStatic = true;
                col.offset = { 0.f,0.f,0.f };
                col.halfExtents = p.scale;      // ★ 半径＝SpawnParams.scale

                return e;
            };
    }

} // namespace Prefabs
