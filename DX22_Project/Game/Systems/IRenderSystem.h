#pragma once
#include "../World.h"

/**
 * @class IRenderSystem
 * @brief 描画処理を担当するシステムの基底クラス
 * @details
 * ECSにおける「System」のうち、**描画（モデル、スプライト、デバッグ線など）**
 * を行うクラスが継承します。
 *
 * Update 系とは分離し、描画順序を制御しやすくします。
 */
class IRenderSystem
{
public:
    /// @brief 仮想デストラクタ（派生クラス破棄時も安全）
    virtual ~IRenderSystem() = default;

    /**
     * @brief 描画処理
     * @param[in] world ECSワールド全体への参照
     * @note 純粋仮想関数。派生クラスで必ず実装する必要があります。
     */
    virtual void Render(const class World& world) = 0;
};
