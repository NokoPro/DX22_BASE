#pragma once
#include <DirectXMath.h>
#include <algorithm>

/**
 * @file AabbUtil.h
 * @brief AABB のワールド化・交差・スイープ判定のユーティリティ群
 * @details
 * - Transform + AabbCollider からワールドAABB(min/max)を生成
 * - 重なり深さ（penetration）計算
 * - Swept AABB: 移動Δを 0..1 の規格化時間で追跡し、最初の衝突時刻/法線を返す
 */

struct Aabb
{
    DirectX::XMFLOAT3 min; ///< 各軸の最小
    DirectX::XMFLOAT3 max; ///< 各軸の最大
};

struct SweptHit
{
    float toi = 1.0f;             ///< time of impact in [0,1]（1は非衝突）
    DirectX::XMFLOAT3 normal{ 0,0,0 }; ///< 衝突面の法線（動的→静的に押し返す向き）
    bool hit = false;
};

/** @brief Transform+ColliderからワールドAABBを作成 */
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

/** @brief AABB同士の重なり深さ（min axis を返す）。重なってなければ (0,0,0) */
inline DirectX::XMFLOAT3 ComputePenetration(const Aabb& a, const Aabb& b)
{
    using namespace DirectX;
    const float dx1 = b.max.x - a.min.x; // b 右端 - a 左端
    const float dx2 = a.max.x - b.min.x; // a 右端 - b 左端
    const float dy1 = b.max.y - a.min.y;
    const float dy2 = a.max.y - b.min.y;
    const float dz1 = b.max.z - a.min.z;
    const float dz2 = a.max.z - b.min.z;

    // 各軸の実重なり量（正なら重なり）
    const float px = std::min(dx1, dx2);
    const float py = std::min(dy1, dy2);
    const float pz = std::min(dz1, dz2);

    if (px <= 0 || py <= 0 || pz <= 0) return { 0,0,0 };

    // 押し出しは最小重なり軸を選ぶのが基本（階段引っかかりを減らす）
    // ただし向きはどちらに押すかで符号が変わるので、ここでは絶対量のみ返す。
    return { px, py, pz };
}

/**
 * @brief Swept AABB（動的AABB vs 静的AABB）
 * @param moving0 開始時刻の動的AABB
 * @param delta   規格化前の移動量（m）
 * @param stat    静的AABB
 * @return 衝突の有無/時刻/法線（未衝突なら hit=false, toi=1）
 * @details
 *  slab法で entry/exit を計算。delta=0軸は「初期から離れていたら非衝突、重なってたら∞幅」とする。
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

    // X軸
    if (dx > 0) {
        t1x = (stat.min.x - moving0.max.x) * invX;
        t2x = (stat.max.x - moving0.min.x) * invX;
    }
    else {
        t1x = (stat.max.x - moving0.min.x) * invX;
        t2x = (stat.min.x - moving0.max.x) * invX;
    }
    // Y軸
    if (dy > 0) {
        t1y = (stat.min.y - moving0.max.y) * invY;
        t2y = (stat.max.y - moving0.min.y) * invY;
    }
    else {
        t1y = (stat.max.y - moving0.min.y) * invY;
        t2y = (stat.min.y - moving0.max.y) * invY;
    }
    // Z軸
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

    // どの軸で入ったか（最大tEntry）で法線を決定
    if (tEntry == std::min(t1x, t2x))      out.normal = { (dx > 0) ? -1.f : +1.f, 0, 0 };
    else if (tEntry == std::min(t1y, t2y)) out.normal = { 0, (dy > 0) ? -1.f : +1.f, 0 };
    else                                  out.normal = { 0, 0, (dz > 0) ? -1.f : +1.f };

    return out;
}
