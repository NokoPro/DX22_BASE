#pragma once
#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <memory>

/**
 * @file ECS.h
 * @brief Entity/Component ��{��`�i�O���錾�̂݁j
 * @details
 * - Entity �͒P�Ȃ鐮��ID
 * - Component �̎��̂� World ���ێ�
 * - System �� World ���󂯎���ē��삷��
 */

 /**
  * @brief �G���e�B�e�BID�^
  * @details
  * 0 �͖����l�Ƃ��ė\��B
  */
using EntityId = uint32_t;

/// �����G���e�B�e�B�萔
constexpr EntityId kInvalidEntity = 0;

/**
 * @brief �R���|�[�l���g�̌^ID��\���w���p�itype_index�j
 */
using ComponentType = std::type_index;

/**
 * @brief �e�R���|�[�l���g�X�g���[�W�̒��ۃC���^�[�t�F�[�X
 */
struct IComponentStorage
{
    virtual ~IComponentStorage() = default;
    virtual void Remove(EntityId e) = 0;
    virtual bool Has(EntityId e) const = 0;
};
