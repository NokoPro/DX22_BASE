#pragma once
#include "../IRenderSystem.h"

/**
 * @file ObbDebugRenderSystem.h
 * @brief ObbCollider �𔼓����̔����� + �΃��C���ŕ`�悷��f�o�b�OSystem
 * @details
 * - Transform + ObbCollider �����S�G���e�B�e�B��`���B
 * - Render(const World&) �����B�\��ON/OFF�͊O���t���O�|�C���^�Ő���B
 */
class ObbDebugRenderSystem final : public IRenderSystem
{
public:
    explicit ObbDebugRenderSystem(const bool* enabled = nullptr)
        : m_enabled(enabled) {
    }

    /// @brief OBB ��`��i�������̓h�� + �֊s���C���j
    void Render(const class World& world) override;

private:
    const bool* m_enabled; ///< �\��ON/OFF�t���O�inullptr�Ȃ��ɕ`��j
};
