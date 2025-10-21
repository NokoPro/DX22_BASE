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
 * @brief �ÓIAABB�i��/�ǂȂǁj�𐶐�����v���n�u
 * @details
 * - SpawnParams.position �� AABB �̒��S�ʒu�ɁASpawnParams.scale ���u�n�[�t�T�C�Y�v�Ƃ��ĉ��߂���B
 * - Render �͕t���Ȃ��i�K�v�Ȃ�ʂ� Render �p�v���n�u��p�Ӂj�B
 */
namespace Prefabs {

    /**
     * @brief �ÓIAABB�p�̃v���n�u�֐������t�@�N�g��
     * @param model �\���Ɏg�����f���inullptr �j
     * @return PrefabRegistry::SpawnFunc�iWorld, SpawnParams -> EntityId�j
     * @note SpawnParams �� scale �� AABB halfExtents �Ƃ��Ĉ����܂��B
     */
    inline PrefabRegistry::SpawnFunc MakeStaticBoxPrefab(Model* model)
    {
        return [model](World& world, const PrefabRegistry::SpawnParams& p) -> EntityId
            {
                const EntityId e = world.Create();

                // 1. TransformComponent (�ʒu�Ɖ�]�𔽉f)
                auto& tr = world.Add<TransformComponent>(e);
                tr.position = {p.position.x, p.position.y,p.position.z - p.scale.z};
                tr.rotationDeg = p.rotationDeg; // <- ��]�𔽉f
                tr.scale = { p.scale.x,p.scale.y,p.scale.z }; // AABB�T�C�Y�� scale �ł͂Ȃ��̂Ō����ڗp�Ƃ���1�Œ�

                // 2. ModelRendererComponent (���f���`��)
                auto& mr = world.Add<ModelRendererComponent>(e);
                mr.model = model;
                mr.visible = true;

                // 3. ObbColliderComponent (�ÓI�Փ�)
                auto& col = world.Add<ObbColliderComponent>(e);
                col.isStatic = true;
                col.offset = { 0.f,0.f,p.scale.z };
                col.halfExtents = p.scale;      // �� AABB�̔����a�� SpawnParams.scale �Ɉˑ�

                return e;
            };
    }

} // namespace Prefabs
