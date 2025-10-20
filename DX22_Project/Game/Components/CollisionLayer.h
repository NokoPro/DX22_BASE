#pragma once
#include <cstdint>

/**
 * @file CollisionLayer.h
 * @brief コリジョングループ／マスク設定用コンポーネント
 * @details
 * - layer: 自身が属するグループ (bit)
 * - mask : 当たる対象のビット集合
 * - 判定条件: (self.mask & other.layer) != 0
 * - ゲーム全体の衝突ルールをここで統一的に管理できる。
 */
struct CollisionLayer {
    enum : uint32_t {
        None = 0,
        Player = 1 << 0,
        Enemy = 1 << 1,
        Level = 1 << 2,
        Sensor = 1 << 3,
        All = 0xFFFFFFFF,
    };

    uint32_t layer = None; ///< 所属レイヤー
    uint32_t mask = All;  ///< 当たる対象

    CollisionLayer(uint32_t l = None, uint32_t m = All)
        : layer(l), mask(m) {
    }
};
