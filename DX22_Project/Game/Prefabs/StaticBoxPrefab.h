#pragma once
#include <DirectXMath.h>
#include "../World.h"
#include "PrefabRegistry.h"

#include "../Components/TransformComponent.h"
#include "../Components/Physics/AabbColliderComponent.h"
#include "../Components/Physics/ObbColliderComponent.h"

/**
 * @file StaticBoxPrefab.h
 * @brief �ÓIAABB�i��/�ǂȂǁj�𐶐�����v���n�u
 * @details
 * - SpawnParams.position �� AABB �̒��S�ʒu�ɁASpawnParams.scale ���u�n�[�t�T�C�Y�v�Ƃ��ĉ��߂���B
 * - Render �͕t���Ȃ��i�K�v�Ȃ�ʂ� Render �p�v���n�u��p�Ӂj�B
 */
namespace Prefabs {

    /**
     * @brief �ÓIAABB�p�̃v���n�u�֐���Ԃ��i�ݒ�͓��ɕs�v�j
     * @return PrefabRegistry::SpawnFunc�iWorld, SpawnParams -> EntityId�j
     * @note SpawnParams �� scale �� AABB halfExtents �Ƃ��Ĉ����܂��B
     */
    inline PrefabRegistry::SpawnFunc MakeStaticBoxPrefab()
    {
        return [](World& world, const PrefabRegistry::SpawnParams& p) -> EntityId
            {
                const EntityId e = world.Create();

                auto& tr = world.Add<TransformComponent>(e);
                tr.position = p.position;
                tr.rotationDeg = p.rotationDeg; // AABB�͎����s�O��Ȃ̂Œʏ��(0,0,0)
                tr.scale = { 1.f,1.f,1.f }; // �����ڗp�ł͂Ȃ��̂�1�Œ�

                auto& col = world.Add<ObbColliderComponent>(e);
                col.isStatic = true;
                col.offset = { 0.f,0.f,0.f };
                col.halfExtents = p.scale;      // �� ���a��SpawnParams.scale

                return e;
            };
    }

} // namespace Prefabs
