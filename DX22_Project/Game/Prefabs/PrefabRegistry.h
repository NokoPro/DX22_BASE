#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>
#include "../World.h"   // EntityId, World

/**
 * @file PrefabRegistry.h
 * @brief �v���n�u�i�G���e�B�e�B�������V�s�j�𖼑O�œo�^�E�������郌�W�X�g��
 * @details
 * - Scene/Systems ����́u���O�{�����v��n���� Spawn() ���邾���B
 * - ���������̎��́i�R���|�[�l���g�t�^�Ȃǁj�� Prefab ���ɕ����߂�B
 * - C++14�Ή��Fstd::function �ŃV���v���Ɏ����B
 */
class PrefabRegistry {
public:
    /**
     * @brief �v���n�u�������ɓn���p�����[�^
     * @details
     * - �ʒu�E��][deg]�E�X�P�[����W�����B�K�v�ɉ����Ċg���\�B
     */
    struct SpawnParams {
        DirectX::XMFLOAT3 position{ 0,0,0 };
        DirectX::XMFLOAT3 rotationDeg{ 0,0,0 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
    };

    /// �����֐��FWorld �� SpawnParams ���󂯎��A�쐬���� EntityId ��Ԃ�
    using SpawnFunc = std::function<EntityId(World&, const SpawnParams&)>;

public:
    PrefabRegistry() {}

    /**
     * @brief �v���n�u��o�^
     * @param name ���ʖ��i��F"Player" �� "StaticBox"�j
     * @param fn   �����֐��iWorld, SpawnParams -> EntityId�j
     * @details
     * - ���������ɂ���Ώ㏑���B
     */
    void Register(const std::string& name, SpawnFunc fn);

    /**
     * @brief �v���n�u�𐶐��i�X�|�[���j
     * @param name ���ʖ�
     * @param world ECS���[���h
     * @param params �����p�����[�^
     * @return �������ꂽ EntityId�i���s���� kInvalidEntity�j
     */
    EntityId Spawn(const std::string& name, World& world, const SpawnParams& params) const;

    /// @brief �o�^�L���̊m�F
    bool Has(const std::string& name) const;

private:
    std::unordered_map<std::string, SpawnFunc> m_map;
};
