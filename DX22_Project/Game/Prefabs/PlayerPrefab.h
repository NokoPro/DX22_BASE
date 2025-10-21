#pragma once
#include <DirectXMath.h>
#include "../World.h"
#include "PrefabRegistry.h"

// === �K�v�R���|�[�l���g ===
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
 * @brief Player �G���e�B�e�B�𐶐�����v���n�u�i�������V�s�j
 * @details
 * - Transform / ModelRenderer / CharacterController / Rigidbody /
 *   AabbCollider / Grounding / MotionDelta ���ꊇ�t�^����B
 * - Model* �͊O���iScene �Ȃǁj�̎��Y���L���v�`�����ēn���B
 */
namespace Prefabs {

    /// �v���C���[�̒����p�p�����[�^
    struct PlayerConfig {
        Model* model = nullptr;                         ///< �\���Ɏg�����f��
        DirectX::XMFLOAT3 colliderHalf{ 0.2f,0.3f,0.2f }; ///< AABB�n�[�t�T�C�Y
        DirectX::XMFLOAT3 colliderOffset{ 0.f,0.3f,0.f }; ///< AABB�I�t�Z�b�g
        float inverseMass = 1.0f;                    ///< 1/mass�i0�ŐÓI�j
        float linearDamping = 0.05f;                   ///< ���x����
        bool  gravityEnabled = true;                    ///< �d��ON/OFF
        float gravityScale = 1.0f;                    ///< �d�͔{��
    };

    /**
     * @brief Player �p�v���n�u�����֐������t�@�N�g��
     * @param cfg �����p�����[�^�iModel* �܂ށj
     * @return PrefabRegistry::SpawnFunc�iWorld, SpawnParams -> EntityId�j
     * @details
     * - SpawnParams.position/rotationDeg/scale �� Transform �ɔ��f�B
     * - scale �̓��f���̌����ڗp�iAABB�T�C�Y�� cfg.colliderHalf ���g���j�B
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
                mr.model = cfg.model;   // null �ł��������͉̂\
                mr.visible = true;

                // Input�i�Ӑ}�j
                world.Add<CharacterControllerComponent>(e);

                // Rigidbody�i�����j
                auto& rb = world.Add<RigidbodyComponent>(e);
                rb.inverseMass = cfg.inverseMass;
                rb.linearDamping = cfg.linearDamping;
                rb.gravityEnabled = cfg.gravityEnabled;
                rb.gravityScale = cfg.gravityScale;

                // OBB�i���I�j
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
