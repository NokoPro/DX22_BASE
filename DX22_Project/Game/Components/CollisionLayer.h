#pragma once
#include <cstdint>

/**
 * @file CollisionLayer.h
 * @brief �R���W�����O���[�v�^�}�X�N�ݒ�p�R���|�[�l���g
 * @details
 * - layer: ���g��������O���[�v (bit)
 * - mask : ������Ώۂ̃r�b�g�W��
 * - �������: (self.mask & other.layer) != 0
 * - �Q�[���S�̂̏Փ˃��[���������œ���I�ɊǗ��ł���B
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

    uint32_t layer = None; ///< �������C���[
    uint32_t mask = All;  ///< ������Ώ�

    CollisionLayer(uint32_t l = None, uint32_t m = All)
        : layer(l), mask(m) {
    }
};
