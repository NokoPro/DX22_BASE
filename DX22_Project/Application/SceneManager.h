#pragma once
#include <memory>
#include "../System/Scene.h"

/**
 * @file SceneManager.h
 * @brief 単一アクティブシーンを管理し、切替・更新・描画を仲介する管理クラス
 * @details
 * - 本管理は「常に1つだけ」アクティブな Scene を保持する。
 * - Change<T>(args...) でシーン切替（古いシーンは破棄）。
 * - Update()/Draw() は Scene の RootUpdate()/RootDraw() 経由で呼ぶ。
 */
class SceneManager
{
public:
    SceneManager() = default;
    ~SceneManager() = default;

    /** @brief シーン切替（古いシーンは破棄） */
    template<class T, class... Args>
    void Change(Args&&... args) {
        static_assert(std::is_base_of<Scene, T>::value, "T must derive from Scene");
        m_current.reset(new T(std::forward<Args>(args)...));
    }

    /** @brief 現在のシーンを更新 */
    void Update() {
        if (m_current) m_current->RootUpdate();
    }

    /** @brief 現在のシーンを描画 */
    void Draw() {
        if (m_current) m_current->RootDraw();
    }

    /** @brief 現在のシーン取得（必要ならキャストして使う） */
    Scene* Current() const { return m_current.get(); }

private:
    std::unique_ptr<Scene> m_current; ///< アクティブシーン
};
