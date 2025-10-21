#pragma once
#include "../IUpdateSystem.h"
#include <vector>
#include <DirectXMath.h>
#include "../Physics/ObbUtil.h"

/**
 * @file CollisionSystemOBB.h
 * @brief ���IOBB��ÓIOBB�ɑ΂��āu�X�C�[�v�i�A������j�v���A�ڐG��O�Œ�~�{�X���C�h����System
 * @details
 * - ���́FTransform, Rigidbody, MotionDelta, ObbCollider�i���I�j�^ Transform, ObbCollider�i�ÓI�j
 * - �菇�F
 *   1) �ÓIOBB�����O���W�i�L���b�V���j
 *   2) �e���I�ɑ΂��Aremain(��]�ړ���) �������ăX�C�[�v�O�i���q�b�g���͐ڐG���O�ֈړ�
 *   3) �@�������́u�ǂɌ��������x���������v���������A�c��ړ���@���������ăX���C�h
 *   4) groundNormalY 臒l�Őڒn������X�V
 *   5) �ŏI�I�� md.delta ������s����
 */
class CollisionSystemOBB final : public IUpdateSystem
{
public:
    /** @brief �����񐔂�X�L�����Ȃǂ̊���l��ݒ� */
    explicit CollisionSystemOBB(int sweepIterations, float sweepSlop)
        : m_sweepIterations(sweepIterations)
        , m_sweepSlop(sweepSlop)
        , m_skin(0.01f)
        , m_groundNormalY(0.5f)
    {
    }

    void Update(class World& world, float dt) override;

private:
    int   m_sweepIterations;  ///< 1�t���[�����ł̍ő�O�i�����񐔁i�X���C�h�܂ށj
    float m_sweepSlop;        ///< 0�Ƃ݂Ȃ����e������
    float m_skin;             ///< �ڐG��O�Ŏ~�߂�X�L����
    float m_groundNormalY;    ///< �ڒn����̖@�� y 臒l

    struct StaticObbCache {
        WorldObb obb;
    };
    std::vector<StaticObbCache> m_staticObbs;

    void BuildStaticObbCache(class World& world);
};
