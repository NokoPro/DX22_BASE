#pragma once
#include <DirectXMath.h>
#include <algorithm>
#include <cmath>

/**
 * @file ObbUtil.h
 * @brief OBB�̃��[���h���A�s�񐶐��A����������(SAT)���[�e�B���e�B
 */

struct WorldObb
{
    DirectX::XMFLOAT3 center;   ///< ���[���h���S
    DirectX::XMFLOAT3 axis[3];  ///< ���𐳋K���ς݃��[���h���iX,Y,Z�j
    DirectX::XMFLOAT3 half;     ///< ���a�i�e�������j
};

//////////////////////////////////////////////////////////////////////////
// �ϊ��E�s�񐶐�
//////////////////////////////////////////////////////////////////////////

inline DirectX::XMMATRIX MakeRotXYZ(float rxDeg, float ryDeg, float rzDeg)
{
    using namespace DirectX;
    const float rx = XMConvertToRadians(rxDeg);
    const float ry = XMConvertToRadians(ryDeg);
    const float rz = XMConvertToRadians(rzDeg);
    return XMMatrixRotationX(rx) * XMMatrixRotationY(ry) * XMMatrixRotationZ(rz);
}

/** @brief Transform + ObbCollider ���� WorldObb �𐶐� */
template<class TTransform, class TObb>
inline WorldObb MakeWorldObb(const TTransform& tr, const TObb& obb)
{
    using namespace DirectX;

    // 1) ��]�FTransform�̉�] * OBB���[�J����]
    const XMMATRIX Rtr = MakeRotXYZ(tr.rotationDeg.x, tr.rotationDeg.y, tr.rotationDeg.z);
    const XMMATRIX Robb = MakeRotXYZ(obb.rotationDeg.x, obb.rotationDeg.y, obb.rotationDeg.z);
    const XMMATRIX R = Rtr * Robb;

    // 2) ���S�FTransform.position + Rtr * obb.offset
    XMVECTOR c = XMLoadFloat3(&tr.position);
    XMVECTOR o = XMLoadFloat3(&obb.offset);
    XMVECTOR cWorld = c + XMVector3TransformNormal(o, Rtr);

    // 3) ���[���h���FR �̊e��x�N�g�� �� ���K��
    XMFLOAT3 ax[3];
    XMStoreFloat3(&ax[0], XMVector3Normalize(R.r[0])); // X��
    XMStoreFloat3(&ax[1], XMVector3Normalize(R.r[1])); // Y��
    XMStoreFloat3(&ax[2], XMVector3Normalize(R.r[2])); // Z��

    WorldObb out;
    XMStoreFloat3(&out.center, cWorld);
    out.axis[0] = ax[0];
    out.axis[1] = ax[1];
    out.axis[2] = ax[2];
    out.half = obb.halfExtents;
    return out;
}

/** @brief WorldObb ���烏�[���h�s��(�]�u)�����i�`��p�j */
inline DirectX::XMFLOAT4X4 MakeWorldMatrixT(const WorldObb& w)
{
    using namespace DirectX;
    // ��x�N�g�� = �X�P�[���ςݎ�
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
// OBB vs OBB ��SAT�i�������j���� + �N���[��
//////////////////////////////////////////////////////////////////////////

struct ObbHit {
    bool hit = false;
    DirectX::XMFLOAT3 normal{ 0,0,0 }; ///< A�ɑ΂��鉟���o���@���iA��B�j
    float depth = 0.f;                ///< �ŏ��N���[
};

/** @brief �x�N�g���̐�Βl�i�e������abs�j */
inline DirectX::XMFLOAT3 Abs3(const DirectX::XMFLOAT3& v)
{
    using namespace DirectX;
    return { std::fabs(v.x), std::fabs(v.y), std::fabs(v.z) };
}

/** @brief 3x3 �s��̐�Βl�i�e����abs�j */
inline void AbsMat3(const float M[3][3], float A[3][3])
{
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) A[i][j] = std::fabs(M[i][j]);
}

/**
 * @brief OBB vs OBB ��SAT�i15���j�ŏՓ˔���ƍŏ��N���@�������߂�
 * @param A ����A
 * @param B ����B
 * @return ObbHit�ihit=false�Ȃ��Փˁj
 * @note �Q�l�FGottschalk �gOBBTree�h / Christer Ericson �gReal-Time Collision Detection�h
 */
inline ObbHit IntersectOBB(const WorldObb& A, const WorldObb& B)
{
    using namespace DirectX;

    // ���x�N�g��
    XMVECTOR Ax = XMLoadFloat3(&A.axis[0]);
    XMVECTOR Ay = XMLoadFloat3(&A.axis[1]);
    XMVECTOR Az = XMLoadFloat3(&A.axis[2]);
    XMVECTOR Bx = XMLoadFloat3(&B.axis[0]);
    XMVECTOR By = XMLoadFloat3(&B.axis[1]);
    XMVECTOR Bz = XMLoadFloat3(&B.axis[2]);

    // ���S�ԃx�N�g��
    XMVECTOR t = XMLoadFloat3(&B.center) - XMLoadFloat3(&A.center);

    // R = A����B���̃h�b�g
    float Rm[3][3] = {
        { XMVectorGetX(XMVector3Dot(Ax,Bx)), XMVectorGetX(XMVector3Dot(Ax,By)), XMVectorGetX(XMVector3Dot(Ax,Bz)) },
        { XMVectorGetX(XMVector3Dot(Ay,Bx)), XMVectorGetX(XMVector3Dot(Ay,By)), XMVectorGetX(XMVector3Dot(Ay,Bz)) },
        { XMVectorGetX(XMVector3Dot(Az,Bx)), XMVectorGetX(XMVector3Dot(Az,By)), XMVectorGetX(XMVector3Dot(Az,Bz)) },
    };
    float AbsR[3][3]; AbsMat3(Rm, AbsR);

    // t ��A�����ɕ\��
    float tA[3] = {
        XMVectorGetX(XMVector3Dot(t, Ax)),
        XMVectorGetX(XMVector3Dot(t, Ay)),
        XMVectorGetX(XMVector3Dot(t, Az))
    };

    const XMFLOAT3 a = A.half;
    const XMFLOAT3 b = B.half;

    auto testAxis = [&](float dist, float ra, float rb, XMFLOAT3 n, ObbHit& best)->bool {
        const float overlap = ra + rb - std::fabs(dist);
        if (overlap < 0.f) return false;
        if (!best.hit || overlap < best.depth) {
            best.hit = true; best.depth = overlap; best.normal = n;
        }
        return true;
        };

    ObbHit best{ false,{0,0,0},0 };

    // 1) A�̎�
    for (int i = 0; i < 3; ++i) {
        const float dist = std::fabs(tA[i]);
        const float ra = (&a.x)[i];
        const float rb = b.x * AbsR[i][0] + b.y * AbsR[i][1] + b.z * AbsR[i][2];
        XMFLOAT3 n = { 0,0,0 }; (&n.x)[i] = (tA[i] < 0.f ? -1.f : +1.f);
        if (!testAxis(tA[i], ra, rb, n, best)) return { false,{0,0,0},0 };
    }

    // 2) B�̎�
    for (int j = 0; j < 3; ++j) {
        const float dist = std::fabs(tA[0] * Rm[0][j] + tA[1] * Rm[1][j] + tA[2] * Rm[2][j]);
        const float ra = a.x * AbsR[0][j] + a.y * AbsR[1][j] + a.z * AbsR[2][j];
        const float rb = (&b.x)[j];
        XMFLOAT3 n = { 0,0,0 };
        // �@����A����B�������ցFsign��A��B
        if (Rm[0][j] * tA[0] + Rm[1][j] * tA[1] + Rm[2][j] * tA[2] < 0.f) (&n.x)[j] = -1.f; else (&n.x)[j] = +1.f;
        if (!testAxis(dist, ra, rb, n, best)) return { false,{0,0,0},0 };
    }

    // 3) ������ A_i x B_j�i���l����̂��ߏ�����R��0�����ł��j
    auto crossN = [&](XMVECTOR ai, XMVECTOR bj)->DirectX::XMFLOAT3 {
        XMFLOAT3 out; XMStoreFloat3(&out, XMVector3Normalize(XMVector3Cross(ai, bj)));
        return out;
        };
    const XMVECTOR Aaxis[3] = { Ax,Ay,Az };
    const XMVECTOR Baxis[3] = { Bx,By,Bz };

    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
    {
        // ra, rb �iEricson�̎��j
        const float ra =
            (&a.x)[i == 0 ? 1 : 0] * AbsR[(i == 0 ? 1 : 0)][j] +
            (&a.x)[i == 2 ? 1 : 2] * AbsR[(i == 2 ? 1 : 2)][j];
        const float rb =
            (&b.x)[j == 0 ? 1 : 0] * AbsR[i][(j == 0 ? 1 : 0)] +
            (&b.x)[j == 2 ? 1 : 2] * AbsR[i][(j == 2 ? 1 : 2)];

        // t �̊Y������
        // |(t / (A_i �~ B_j))| �� A���ŋߎ��i�����ȗ��A���p����Ȃ��j
        const XMVECTOR N = XMVector3Cross(Aaxis[i], Baxis[j]);
        const float dist = std::fabs(XMVectorGetX(XMVector3Dot(t, N)));

        XMFLOAT3 n = crossN(Aaxis[i], Baxis[j]);
        if (std::isfinite(n.x + n.y + n.z) == false) continue; // �قڕ��s

        if (!testAxis(dist, ra, rb, n, best)) return { false,{0,0,0},0 };
    }

    return best;
}
