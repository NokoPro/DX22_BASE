#pragma once
#include "../IRenderSystem.h"

/**
 * @file DebugGridRenderSystem.h
 * @brief XZ���ʂ̃O���b�h�Ǝ���`�悷�邾���̃f�o�b�O�p�����_�����O�V�X�e��
 * @details
 * - F1�g�O�����̊O���t���O�|�C���^���󂯎��A����ɏ]���ĕ`��B
 */
class DebugGridRenderSystem final : public IRenderSystem
{
public:
    explicit DebugGridRenderSystem(bool* flagPtr) : m_flag(flagPtr) {}
    void Render(const class World& world) override;  // �� Draw��Render / const World& �ɍ��킹��

private:
    bool* m_flag; ///< �\���t���O�i�O���Ǘ��j
};
