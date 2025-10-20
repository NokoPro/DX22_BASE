#pragma once
#include <functional>
#include <unordered_map>
#include <string>
#include <DirectXMath.h>
#include "../World.h"   // EntityId, World

/**
 * @file PrefabRegistry.h
 * @brief プレハブ（エンティティ生成レシピ）を名前で登録・生成するレジストリ
 * @details
 * - Scene/Systems からは「名前＋引数」を渡して Spawn() するだけ。
 * - 生成処理の実体（コンポーネント付与など）は Prefab 側に閉じ込める。
 * - C++14対応：std::function でシンプルに実装。
 */
class PrefabRegistry {
public:
    /**
     * @brief プレハブ生成時に渡すパラメータ
     * @details
     * - 位置・回転[deg]・スケールを標準化。必要に応じて拡張可能。
     */
    struct SpawnParams {
        DirectX::XMFLOAT3 position{ 0,0,0 };
        DirectX::XMFLOAT3 rotationDeg{ 0,0,0 };
        DirectX::XMFLOAT3 scale{ 1,1,1 };
    };

    /// 生成関数：World と SpawnParams を受け取り、作成した EntityId を返す
    using SpawnFunc = std::function<EntityId(World&, const SpawnParams&)>;

public:
    PrefabRegistry() {}

    /**
     * @brief プレハブを登録
     * @param name 識別名（例："Player" や "StaticBox"）
     * @param fn   生成関数（World, SpawnParams -> EntityId）
     * @details
     * - 同名が既にあれば上書き。
     */
    void Register(const std::string& name, SpawnFunc fn);

    /**
     * @brief プレハブを生成（スポーン）
     * @param name 識別名
     * @param world ECSワールド
     * @param params 生成パラメータ
     * @return 生成された EntityId（失敗時は kInvalidEntity）
     */
    EntityId Spawn(const std::string& name, World& world, const SpawnParams& params) const;

    /// @brief 登録有無の確認
    bool Has(const std::string& name) const;

private:
    std::unordered_map<std::string, SpawnFunc> m_map;
};
