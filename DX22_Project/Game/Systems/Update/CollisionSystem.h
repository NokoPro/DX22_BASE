#pragma once
#include "../IUpdateSystem.h"
#include <vector>
#include <DirectXMath.h>
#include "../Physics/AabbUtil.h"

/**
 * @file CollisionSystem.h
 * @brief MotionDelta ��ÓIAABB�ɑ΂��ĉ������ATransform ���m��E�ڒn�X�V����System
 * @details
 * - ���́FTransform, Rigidbody, MotionDelta, AabbCollider�i���I�j�^ Transform, AabbCollider�i�ÓI�j
 * - �菇�F
 *   1) �ÓIAABB�����O���W
 *   2) �e���I�ɑ΂��āu�X�C�[�v�iSweptAabb�j�� �����o���i�d�Ȃ�j�� ���x�C�� �� �ڒn�X�V�v
 *   3) �ŏI Transform.position ������idelta ������j
 * - �A���Փ˂ɔ����A�ő唽���񐔂�݂���i�f�t�H 3�j�B
 */
class CollisionSystem final : public IUpdateSystem {
public:
    explicit CollisionSystem(int maxIterations = 3, float groundNormalY = 0.6f)
        : m_maxIterations(maxIterations), m_groundNormalY(groundNormalY) {
    }

    void Update(class World& world, float dt) override;

private:
    int   m_maxIterations; ///< 1�t���[�����̍ő唽����
    float m_groundNormalY; ///< ����ȏ�Ȃ�u��ʁv�Ƃ݂Ȃ��Đڒn����

    struct StaticAabbCache 
    {
        Aabb aabb;
    };

    std::vector<StaticAabbCache> m_staticAabbs;

    void BuildStaticAabbCache(class World& world);
};
