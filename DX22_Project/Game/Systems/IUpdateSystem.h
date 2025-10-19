#pragma once
#include "../World.h"  // Worldを使うなら必要。前方宣言でも可。

/**
 * @class IUpdateSystem
 * @brief 毎フレームの更新処理を担当するシステムの基底クラス
 * @details
 * ECSにおける「System」のうち、**ロジック更新（物理・入力・アニメーションなど）**
 * を行うクラスが継承します。
 *
 * すべての派生クラスは Update() を実装し、
 * World 内のコンポーネント群を参照・変更します。
 */
class IUpdateSystem
{
public:
    /// @brief 仮想デストラクタ（派生クラス破棄時も安全）
    virtual ~IUpdateSystem() = default;

    /**
     * @brief 更新処理
     * @param[in,out] world ECSワールド全体への参照
     * @param[in] dt 前フレームからの経過時間（秒）
     * @note 純粋仮想関数。派生クラスで必ず実装する必要があります。
     */
    virtual void Update(class World& world, float dt) = 0;
};