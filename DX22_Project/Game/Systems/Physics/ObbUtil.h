#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

/**
 * @file ObbUtil.h
 * @brief OBBのワールド化、行列生成、分離軸判定(SAT)ユーティリティ
 */

struct WorldObb
{
    DirectX::XMFLOAT3 center;   ///< ワールド中心
    DirectX::XMFLOAT3 axis[3];  ///< 直交正規化済みワールド軸（X,Y,Z）
    DirectX::XMFLOAT3 half;     ///< 半径（各軸方向）
};

//////////////////////////////////////////////////////////////////////////
// 変換・行列生成
//////////////////////////////////////////////////////////////////////////

inline DirectX::XMMATRIX MakeRotXYZ(float rxDeg, float ryDeg, float rzDeg)
{
    using namespace DirectX;
    const float rx = XMConvertToRadians(rxDeg);
    const float ry = XMConvertToRadians(ryDeg);
    const float rz = XMConvertToRadians(rzDeg);
    return XMMatrixRotationX(rx) * XMMatrixRotationY(ry) * XMMatrixRotationZ(rz);
}

/** @brief Transform + ObbCollider から WorldObb を生成 */
template<class TTransform, class TObb>
inline WorldObb MakeWorldObb(const TTransform& tr, const TObb& obb)
{
    using namespace DirectX;

    // 1) 回転：Transformの回転 * OBBローカル回転
    const XMMATRIX Rtr = MakeRotXYZ(tr.rotationDeg.x, tr.rotationDeg.y, tr.rotationDeg.z);
    const XMMATRIX Robb = MakeRotXYZ(obb.rotationDeg.x, obb.rotationDeg.y, obb.rotationDeg.z);
    const XMMATRIX R = Rtr * Robb;

    // 2) 中心：Transform.position + R * obb.offset   ← ★ Rtr → R に修正
    XMVECTOR c = XMLoadFloat3(&tr.position);
    XMVECTOR o = XMLoadFloat3(&obb.offset);
    XMVECTOR cWorld = c + XMVector3TransformNormal(o, R);

    // 3) ワールド軸：単位軸(1,0,0)/(0,1,0)/(0,0,1)を R で回して正規化  ← ★ 行/列の混乱を回避
    XMFLOAT3 ax[3];
    XMStoreFloat3(&ax[0], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), R)));
    XMStoreFloat3(&ax[1], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), R)));
    XMStoreFloat3(&ax[2], XMVector3Normalize(XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), R)));

    WorldObb out;
    XMStoreFloat3(&out.center, cWorld);
    out.axis[0] = ax[0];
    out.axis[1] = ax[1];
    out.axis[2] = ax[2];
    out.half = obb.halfExtents;
    return out;
}

/** @brief WorldObb からワールド行列(転置)を作る（描画用） */
inline DirectX::XMFLOAT4X4 MakeWorldMatrixT(const WorldObb& w)
{
    using namespace DirectX;
    // 列ベクトル = スケール済み軸
    const XMVECTOR ax = XMVectorSet(w.axis[0].x, w.axis[0].y, w.axis[0].z, 0);
    const XMVECTOR ay = XMVectorSet(w.axis[1].x, w.axis[1].y, w.axis[1].z, 0);
    const XMVECTOR az = XMVectorSet(w.axis[2].x, w.axis[2].y, w.axis[2].z, 0);

    const XMMATRIX A = XMMATRIX(
        XMVectorScale(ax, w.half.x * 2.0f),
        XMVectorScale(ay, w.half.y * 2.0f),
        XMVectorScale(az, w.half.z * 2.0f),
        XMVectorZero());

    const XMMATRIX T = XMMatrixTranslation(w.center.x, w.center.y, w.center.z);
    const XMMATRIX W = A * T;

    XMFLOAT4X4 Wt;
    XMStoreFloat4x4(&Wt, XMMatrixTranspose(W));
    return Wt;
}

//////////////////////////////////////////////////////////////////////////
// OBB vs OBB のSAT（分離軸）判定 + 侵入深さ
//////////////////////////////////////////////////////////////////////////

struct ObbHit {
    bool hit = false;
    DirectX::XMFLOAT3 normal{ 0,0,0 }; ///< Aに対する押し出し法線（A←B）
    float depth = 0.f;                ///< 最小侵入深
};

/** @brief ベクトルの絶対値（各成分のabs） */
inline DirectX::XMFLOAT3 Abs3(const DirectX::XMFLOAT3& v)
{
    using namespace DirectX;
    return { std::fabs(v.x), std::fabs(v.y), std::fabs(v.z) };
}

/** @brief 3x3 行列の絶対値（各成分abs） */
inline void AbsMat3(const float M[3][3], float A[3][3])
{
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) A[i][j] = std::fabs(M[i][j]);
}

/**
 * @brief OBB vs OBB のSAT（15軸）で衝突判定と最小侵入法線を求める
 * @param A 物体A
 * @param B 物体B
 * @return ObbHit（hit=falseなら非衝突）
 * @note 参考：Gottschalk “OBBTree” / Christer Ericson “Real-Time Collision Detection”
 */
inline ObbHit IntersectOBB(const WorldObb& A, const WorldObb& B)
{
    using namespace DirectX;

    // A軸・B軸（単位）
    XMVECTOR Ax = XMLoadFloat3(&A.axis[0]);
    XMVECTOR Ay = XMLoadFloat3(&A.axis[1]);
    XMVECTOR Az = XMLoadFloat3(&A.axis[2]);
    XMVECTOR Bx = XMLoadFloat3(&B.axis[0]);
    XMVECTOR By = XMLoadFloat3(&B.axis[1]);
    XMVECTOR Bz = XMLoadFloat3(&B.axis[2]);

    // 中心差分（A基底での t）
    XMVECTOR tWorld = XMLoadFloat3(&B.center) - XMLoadFloat3(&A.center);
    float tA[3] = {
        XMVectorGetX(XMVector3Dot(tWorld, Ax)),
        XMVectorGetX(XMVector3Dot(tWorld, Ay)),
        XMVectorGetX(XMVector3Dot(tWorld, Az))
    };

    // R = A_i ・ B_j
    float Rm[3][3] = {
        { XMVectorGetX(XMVector3Dot(Ax,Bx)), XMVectorGetX(XMVector3Dot(Ax,By)), XMVectorGetX(XMVector3Dot(Ax,Bz)) },
        { XMVectorGetX(XMVector3Dot(Ay,Bx)), XMVectorGetX(XMVector3Dot(Ay,By)), XMVectorGetX(XMVector3Dot(Ay,Bz)) },
        { XMVectorGetX(XMVector3Dot(Az,Bx)), XMVectorGetX(XMVector3Dot(Az,By)), XMVectorGetX(XMVector3Dot(Az,Bz)) },
    };

    // |R| + ε（平行時の数値不安定を回避）
    const float EPS = 1e-6f;
    float AbsR[3][3];
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
        AbsR[i][j] = std::fabs(Rm[i][j]) + EPS;

    const XMFLOAT3 a = A.half;
    const XMFLOAT3 b = B.half;

    auto updateBest = [&](float sep, XMVECTOR axis, ObbHit& best)->bool {
        // sep < 0 なら分離；>=0 なら重なり。ここでは重なっている時の最小侵入量を最小化
        if (sep < 0.f) return false; // 分離
        if (!best.hit || sep < best.depth) {
            best.hit = true;
            best.depth = sep;
            // ワールド向きの法線（A←B 方向に押し出す）
            XMVECTOR n = XMVector3Normalize(axis);
            // tWorld と法線の符号で向きを合わせる（外側へ）
            if (XMVectorGetX(XMVector3Dot(n, tWorld)) > 0.f) n = XMVectorNegate(n);
            XMStoreFloat3(&best.normal, n);
        }
        return true;
        };

    ObbHit best{ false,{0,0,0},0 };

    // 1) Aの軸
    for (int i = 0; i < 3; ++i) {
        float ra = (&a.x)[i];
        float rb = b.x * AbsR[i][0] + b.y * AbsR[i][1] + b.z * AbsR[i][2];
        float sep = (ra + rb) - std::fabs(tA[i]);
        XMVECTOR axis = (i == 0 ? Ax : (i == 1 ? Ay : Az));
        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    // 2) Bの軸
    for (int j = 0; j < 3; ++j) {
        float ra = a.x * AbsR[0][j] + a.y * AbsR[1][j] + a.z * AbsR[2][j];
        float rb = (&b.x)[j];
        // t をB基底に：tB[j] = dot(tA, R_col_j)
        float tBj = std::fabs(tA[0] * Rm[0][j] + tA[1] * Rm[1][j] + tA[2] * Rm[2][j]);
        XMVECTOR axis = (j == 0 ? Bx : (j == 1 ? By : Bz));
        float sep = (ra + rb) - tBj;
        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    // 3) 交差軸 Ai x Bj
    const XMVECTOR Aaxis[3] = { Ax,Ay,Az };
    const XMVECTOR Baxis[3] = { Bx,By,Bz };

    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
        // ra = a[(i+1)%3]*|R[(i+2)%3][j]| + a[(i+2)%3]*|R[(i+1)%3][j]|
        float ra = (&a.x)[(i + 1) % 3] * AbsR[(i + 2) % 3][j] + (&a.x)[(i + 2) % 3] * AbsR[(i + 1) % 3][j];
        // rb = b[(j+1)%3]*|R[i][(j+2)%3]| + b[(j+2)%3]*|R[i][(j+1)%3]|
        float rb = (&b.x)[(j + 1) % 3] * AbsR[i][(j + 2) % 3] + (&b.x)[(j + 2) % 3] * AbsR[i][(j + 1) % 3];
        // |t ・ (Ai × Bj)| を A基底での式に展開
        float tProj = std::fabs(tA[(i + 2) % 3] * Rm[(i + 1) % 3][j] - tA[(i + 1) % 3] * Rm[(i + 2) % 3][j]);
        float sep = (ra + rb) - tProj;

        // 法線 = Ai × Bj（ワールド）
        XMVECTOR axis = XMVector3Cross(Aaxis[i], Baxis[j]);
        // 平行に近い場合はスキップ
        if (XMVectorGetX(XMVector3LengthSq(axis)) < 1e-12f) continue;

        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    return best;
}

/**
 * @brief レイヒット結果（スイープ判定用）
 * - t: 0～1 の範囲（フレーム中の衝突時刻） / normal: ワールド法線
 */
struct RayHit {
    bool hit = false;
    float t = 1.0f;                 // 0..1
    DirectX::XMFLOAT3 normal{ 0,0,0 }; // world-space
};

/**
 * @brief 点レイ vs 「動的OBBで拡張した静的OBB(AABB化)」
 * @param stat 静的OBB（ワールド）
 * @param dyn  動的OBB（ワールド）※拡張量の算出に使用
 * @param p0   レイ開始点（ワールド）
 * @param v    レイ方向（フレーム移動量、ワールド）
 * @param skin 接触時に手前へ戻す距離（※AABB拡張には使わない）
 */
inline RayHit RaycastPointVsExpandedObb(const WorldObb& stat, const WorldObb& dyn,
    const DirectX::XMFLOAT3& p0,
    const DirectX::XMFLOAT3& v,
    float /*skin*/)
{
    using namespace DirectX;

    // stat軸/ dyn軸
    XMFLOAT3 uS[3] = { stat.axis[0], stat.axis[1], stat.axis[2] };
    XMFLOAT3 uD[3] = { dyn.axis[0],  dyn.axis[1],  dyn.axis[2] };

    // |R| = |dot(uS_i, uD_j)|
    float Rabs[3][3];
    for (int i = 0; i < 3; ++i) {
        XMVECTOR si = XMLoadFloat3(&uS[i]);
        for (int j = 0; j < 3; ++j) {
            XMVECTOR dj = XMLoadFloat3(&uD[j]);
            Rabs[i][j] = std::fabs(XMVectorGetX(XMVector3Dot(si, dj)));
        }
    }

    // 拡張量 ext = |R| * dyn.half
    XMFLOAT3 extExp = {
        Rabs[0][0] * dyn.half.x + Rabs[0][1] * dyn.half.y + Rabs[0][2] * dyn.half.z,
        Rabs[1][0] * dyn.half.x + Rabs[1][1] * dyn.half.y + Rabs[1][2] * dyn.half.z,
        Rabs[2][0] * dyn.half.x + Rabs[2][1] * dyn.half.y + Rabs[2][2] * dyn.half.z
    };

    // ★skinはAABB半径に入れない
    XMFLOAT3 A = {
        stat.half.x + extExp.x,
        stat.half.y + extExp.y,
        stat.half.z + extExp.z
    };

    // ワールド→ statローカル
    auto ToLocal = [&](const XMFLOAT3& w)->XMFLOAT3 {
        XMVECTOR pw = XMLoadFloat3(&w);
        XMVECTOR c = XMLoadFloat3(&stat.center);
        XMVECTOR p = pw - c;
        float x = XMVectorGetX(XMVector3Dot(p, XMLoadFloat3(&uS[0])));
        float y = XMVectorGetX(XMVector3Dot(p, XMLoadFloat3(&uS[1])));
        float z = XMVectorGetX(XMVector3Dot(p, XMLoadFloat3(&uS[2])));
        return { x,y,z };
        };

    XMFLOAT3 pL = ToLocal(p0);
    XMFLOAT3 p1 = { p0.x + v.x, p0.y + v.y, p0.z + v.z };
    XMFLOAT3 vL = ToLocal(p1);
    vL = { vL.x - pL.x, vL.y - pL.y, vL.z - pL.z };

    // スラブ判定
    float tmin = 0.0f, tmax = 1.0f;
    int   hitAxis = -1;
    int   sign = 0;

    auto slab = [&](float p, float d, float a, int axis)->bool {
        const float eps = 1e-8f;
        if (std::fabs(d) < eps) {
            // 平行：外なら非衝突
            return (std::fabs(p) <= a);
        }
        float inv = 1.0f / d;
        float t1 = (-a - p) * inv;
        float t2 = (a - p) * inv;
        int sgn = (t1 > t2);
        if (sgn) std::swap(t1, t2);

        // ★等号も許容して "どこかの軸は必ず記録" できるようにする
        if (t1 >= tmin) { tmin = t1; hitAxis = axis; sign = sgn ? +1 : -1; }
        if (t2 < tmax) tmax = t2;
        return tmin <= tmax;
        };

    if (!slab(pL.x, vL.x, A.x, 0)) return {};
    if (!slab(pL.y, vL.y, A.y, 1)) return {};
    if (!slab(pL.z, vL.z, A.z, 2)) return {};
    if (tmin < 0.0f || tmin > 1.0f) return {};

    // ★安全ガード：ヒット軸が確定していなければヒット無効化（内側スタート等）
    if (hitAxis < 0 || hitAxis > 2) {
        return {};
    }

    // 法線（statローカル→ワールド）
    XMFLOAT3 nLocal{ 0,0,0 };
    (&nLocal.x)[hitAxis] = (sign ? +1.0f : -1.0f);

    // 正規化の前にゼロベクトルを避ける
    XMVECTOR nW =
        XMVectorScale(XMLoadFloat3(&uS[0]), nLocal.x) +
        XMVectorScale(XMLoadFloat3(&uS[1]), nLocal.y) +
        XMVectorScale(XMLoadFloat3(&uS[2]), nLocal.z);

    // 長さチェック（念のため）
    if (XMVector3Equal(nW, XMVectorZero())) {
        return {};
    }

    nW = XMVector3Normalize(nW);

    RayHit h; h.hit = true; h.t = tmin;
    XMStoreFloat3(&h.normal, nW);
    return h;
}
