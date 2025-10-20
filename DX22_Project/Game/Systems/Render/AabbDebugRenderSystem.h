#pragma once
#include "../IRenderSystem.h"

/**
 * @file AabbDebugRenderSystem.h
 * @brief AabbCollider ���u�����{�ڐG���͔��΃��C���v�`�悷��f�o�b�OSystem
 * @details
 * - Transform + AabbCollider �����S�G���e�B�e�B��`���B
 * - Render(const World&) �����B�\��ON/OFF�͊O���t���O�|�C���^�Ő���B
 */
class AabbDebugRenderSystem final : public IRenderSystem {
public:
    /**
     * @param enabled �\��ON/OFF���O�����䂷��t���O�inullptr�j
     */
    explicit AabbDebugRenderSystem(const bool* enabled = nullptr)
        : m_enabled(enabled) {
    }

    /// @brief AABB��`��i�����j�B�ڐG���͔��΃��C�����d�˂�B
    void Render(const class World& world) override;

private:
    const bool* m_enabled; ///< �\��ON/OFF�t���O�inullptr�Ȃ��ɕ`��j
};
