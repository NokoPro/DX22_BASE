#pragma once
#include <cstdint>
#include <typeindex>
#include <unordered_map>
#include <memory>

/**
 * @file ECS.h
 * @brief Entity/Component 基本定義（前方宣言のみ）
 * @details
 * - Entity は単なる整数ID
 * - Component の実体は World が保持
 * - System は World を受け取って動作する
 */

 /**
  * @brief エンティティID型
  * @details
  * 0 は無効値として予約。
  */
using EntityId = uint32_t;

/// 無効エンティティ定数
constexpr EntityId kInvalidEntity = 0;

/**
 * @brief コンポーネントの型IDを表すヘルパ（type_index）
 */
using ComponentType = std::type_index;

/**
 * @brief 各コンポーネントストレージの抽象インターフェース
 */
struct IComponentStorage
{
    virtual ~IComponentStorage() = default;
    virtual void Remove(EntityId e) = 0;
    virtual bool Has(EntityId e) const = 0;
};
