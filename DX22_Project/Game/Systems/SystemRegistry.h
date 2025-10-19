#pragma once
#include <memory>
#include <vector>
#include "IUpdateSystem.h"
#include "IRenderSystem.h"

/**
 * @file SystemRegistry.h
 * @brief Update系とRender系を分離実行するレジストリ
 * @details
 * - 登録順 = 実行順（依存関係がある場合は登録順で制御）。
 * - AddUpdate<T>(), AddRender<T>() で構築。 Tick()/Render() で実行。
 */
class SystemRegistry
{
public:
    /** @brief Updateシステムを追加（登録順＝実行順） */
    template<class T, class... Args>
    T& AddUpdate(Args&&... args) {
        static_assert(std::is_base_of<IUpdateSystem, T>::value, "T must be IUpdateSystem");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        m_updates.emplace_back(std::move(ptr));
        return ref;
    }

    /** @brief Renderシステムを追加（登録順＝実行順） */
    template<class T, class... Args>
    T& AddRender(Args&&... args) {
        static_assert(std::is_base_of<IRenderSystem, T>::value, "T must be IRenderSystem");
        auto ptr = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *ptr;
        m_renders.emplace_back(std::move(ptr));
        return ref;
    }

    /** @brief Updateパスを実行 */
    void Tick(class World& world, float dt);

    /** @brief Renderパスを実行 */
    void Render(class World& world);

private:
    std::vector<std::unique_ptr<IUpdateSystem>> m_updates; ///< 更新パス群
    std::vector<std::unique_ptr<IRenderSystem>> m_renders; ///< 描画パス群
};
