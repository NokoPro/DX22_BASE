#pragma once
#include <DirectXMath.h>
#include <algorithm>

/**
 * @file AabbUtil.h
 * @brief AABB �̃��[���h���E�����E�X�C�[�v����̃��[�e�B���e�B�Q
 * @details
 * - Transform + AabbCollider ���烏�[���hAABB(min/max)�𐶐�
 * - �d�Ȃ�[���ipenetration�j�v�Z
 * - Swept AABB: �ړ����� 0..1 �̋K�i�����ԂŒǐՂ��A�ŏ��̏Փˎ���/�@����Ԃ�
 */

struct Aabb
{
    DirectX::XMFLOAT3 min; ///< �e���̍ŏ�
    DirectX::XMFLOAT3 max; ///< �e���̍ő�
};

struct SweptHit
{
    float toi = 1.0f;             ///< time of impact in [0,1]�i1�͔�Փˁj
    DirectX::XMFLOAT3 normal{ 0,0,0 }; ///< �Փ˖ʂ̖@���i���I���ÓI�ɉ����Ԃ������j
    bool hit = false;
};

/** @brief Transform+Collider���烏�[���hAABB���쐬 */
template<class TTransform, class TCollider>
inline Aabb MakeWorldAabb(const TTransform& tr, const TCollider& col)
{
    using namespace DirectX;
    const XMFLOAT3 c{
        tr.position.x + col.offset.x,
        tr.position.y + col.offset.y,
        tr.position.z + col.offset.z
    };
    const XMFLOAT3 he = col.halfExtents;
    return Aabb{
        { c.x - he.x, c.y - he.y, c.z - he.z },
        { c.x + he.x, c.y + he.y, c.z + he.z }
    };
}

/** @brief AABB���m�̏d�Ȃ�[���imin axis ��Ԃ��j�B�d�Ȃ��ĂȂ���� (0,0,0) */
inline DirectX::XMFLOAT3 ComputePenetration(const Aabb& a, const Aabb& b)
{
    using namespace DirectX;
    const float dx1 = b.max.x - a.min.x; // b �E�[ - a ���[
    const float dx2 = a.max.x - b.min.x; // a �E�[ - b ���[
    const float dy1 = b.max.y - a.min.y;
    const float dy2 = a.max.y - b.min.y;
    const float dz1 = b.max.z - a.min.z;
    const float dz2 = a.max.z - b.min.z;

    // �e���̎��d�Ȃ�ʁi���Ȃ�d�Ȃ�j
    const float px = std::min(dx1, dx2);
    const float py = std::min(dy1, dy2);
    const float pz = std::min(dz1, dz2);

    if (px <= 0 || py <= 0 || pz <= 0) return { 0,0,0 };

    // �����o���͍ŏ��d�Ȃ莲��I�Ԃ̂���{�i�K�i��������������炷�j
    // �����������͂ǂ���ɉ������ŕ������ς��̂ŁA�����ł͐�Ηʂ̂ݕԂ��B
    return { px, py, pz };
}

/**
 * @brief Swept AABB�i���IAABB vs �ÓIAABB�j
 * @param moving0 �J�n�����̓��IAABB
 * @param delta   �K�i���O�̈ړ��ʁim�j
 * @param stat    �ÓIAABB
 * @return �Փ˂̗L��/����/�@���i���Փ˂Ȃ� hit=false, toi=1�j
 * @details
 *  slab�@�� entry/exit ���v�Z�Bdelta=0���́u�������痣��Ă������ՓˁA�d�Ȃ��Ă��灇���v�Ƃ���B
 */
inline SweptHit SweptAabb(const Aabb& moving0, const DirectX::XMFLOAT3& delta, const Aabb& stat)
{
    using namespace DirectX;

    const float dx = delta.x, dy = delta.y, dz = delta.z;

    auto inv = [](float d)->float { return (d != 0.f) ? (1.0f / d) : 1e30f; };
    const float invX = inv(dx);
    const float invY = inv(dy);
    const float invZ = inv(dz);

    float t1x, t2x, t1y, t2y, t1z, t2z;

    // X��
    if (dx > 0) {
        t1x = (stat.min.x - moving0.max.x) * invX;
        t2x = (stat.max.x - moving0.min.x) * invX;
    }
    else {
        t1x = (stat.max.x - moving0.min.x) * invX;
        t2x = (stat.min.x - moving0.max.x) * invX;
    }
    // Y��
    if (dy > 0) {
        t1y = (stat.min.y - moving0.max.y) * invY;
        t2y = (stat.max.y - moving0.min.y) * invY;
    }
    else {
        t1y = (stat.max.y - moving0.min.y) * invY;
        t2y = (stat.min.y - moving0.max.y) * invY;
    }
    // Z��
    if (dz > 0) {
        t1z = (stat.min.z - moving0.max.z) * invZ;
        t2z = (stat.max.z - moving0.min.z) * invZ;
    }
    else {
        t1z = (stat.max.z - moving0.min.z) * invZ;
        t2z = (stat.min.z - moving0.max.z) * invZ;
    }

    const float tEntry = std::max(std::max(std::min(t1x, t2x), std::min(t1y, t2y)), std::min(t1z, t2z));
    const float tExit = std::min(std::min(std::max(t1x, t2x), std::max(t1y, t2y)), std::max(t1z, t2z));

    SweptHit out;
    if (tEntry > tExit || tExit < 0.0f || tEntry > 1.0f) {
        out.hit = false; out.toi = 1.0f; out.normal = { 0,0,0 };
        return out;
    }

    out.hit = true;
    out.toi = std::max(0.0f, tEntry);

    // �ǂ̎��œ��������i�ő�tEntry�j�Ŗ@��������
    if (tEntry == std::min(t1x, t2x))      out.normal = { (dx > 0) ? -1.f : +1.f, 0, 0 };
    else if (tEntry == std::min(t1y, t2y)) out.normal = { 0, (dy > 0) ? -1.f : +1.f, 0 };
    else                                  out.normal = { 0, 0, (dz > 0) ? -1.f : +1.f };

    return out;
}
