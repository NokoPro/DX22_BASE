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

    // 2) ���S�FTransform.position + R * obb.offset   �� �� Rtr �� R �ɏC��
    XMVECTOR c = XMLoadFloat3(&tr.position);
    XMVECTOR o = XMLoadFloat3(&obb.offset);
    XMVECTOR cWorld = c + XMVector3TransformNormal(o, R);

    // 3) ���[���h���F�P�ʎ�(1,0,0)/(0,1,0)/(0,0,1)�� R �ŉ񂵂Đ��K��  �� �� �s/��̍��������
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

    // A���EB���i�P�ʁj
    XMVECTOR Ax = XMLoadFloat3(&A.axis[0]);
    XMVECTOR Ay = XMLoadFloat3(&A.axis[1]);
    XMVECTOR Az = XMLoadFloat3(&A.axis[2]);
    XMVECTOR Bx = XMLoadFloat3(&B.axis[0]);
    XMVECTOR By = XMLoadFloat3(&B.axis[1]);
    XMVECTOR Bz = XMLoadFloat3(&B.axis[2]);

    // ���S�����iA���ł� t�j
    XMVECTOR tWorld = XMLoadFloat3(&B.center) - XMLoadFloat3(&A.center);
    float tA[3] = {
        XMVectorGetX(XMVector3Dot(tWorld, Ax)),
        XMVectorGetX(XMVector3Dot(tWorld, Ay)),
        XMVectorGetX(XMVector3Dot(tWorld, Az))
    };

    // R = A_i �E B_j
    float Rm[3][3] = {
        { XMVectorGetX(XMVector3Dot(Ax,Bx)), XMVectorGetX(XMVector3Dot(Ax,By)), XMVectorGetX(XMVector3Dot(Ax,Bz)) },
        { XMVectorGetX(XMVector3Dot(Ay,Bx)), XMVectorGetX(XMVector3Dot(Ay,By)), XMVectorGetX(XMVector3Dot(Ay,Bz)) },
        { XMVectorGetX(XMVector3Dot(Az,Bx)), XMVectorGetX(XMVector3Dot(Az,By)), XMVectorGetX(XMVector3Dot(Az,Bz)) },
    };

    // |R| + �Ái���s���̐��l�s���������j
    const float EPS = 1e-6f;
    float AbsR[3][3];
    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j)
        AbsR[i][j] = std::fabs(Rm[i][j]) + EPS;

    const XMFLOAT3 a = A.half;
    const XMFLOAT3 b = B.half;

    auto updateBest = [&](float sep, XMVECTOR axis, ObbHit& best)->bool {
        // sep < 0 �Ȃ番���G>=0 �Ȃ�d�Ȃ�B�����ł͏d�Ȃ��Ă��鎞�̍ŏ��N���ʂ��ŏ���
        if (sep < 0.f) return false; // ����
        if (!best.hit || sep < best.depth) {
            best.hit = true;
            best.depth = sep;
            // ���[���h�����̖@���iA��B �����ɉ����o���j
            XMVECTOR n = XMVector3Normalize(axis);
            // tWorld �Ɩ@���̕����Ō��������킹��i�O���ցj
            if (XMVectorGetX(XMVector3Dot(n, tWorld)) > 0.f) n = XMVectorNegate(n);
            XMStoreFloat3(&best.normal, n);
        }
        return true;
        };

    ObbHit best{ false,{0,0,0},0 };

    // 1) A�̎�
    for (int i = 0; i < 3; ++i) {
        float ra = (&a.x)[i];
        float rb = b.x * AbsR[i][0] + b.y * AbsR[i][1] + b.z * AbsR[i][2];
        float sep = (ra + rb) - std::fabs(tA[i]);
        XMVECTOR axis = (i == 0 ? Ax : (i == 1 ? Ay : Az));
        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    // 2) B�̎�
    for (int j = 0; j < 3; ++j) {
        float ra = a.x * AbsR[0][j] + a.y * AbsR[1][j] + a.z * AbsR[2][j];
        float rb = (&b.x)[j];
        // t ��B���ɁFtB[j] = dot(tA, R_col_j)
        float tBj = std::fabs(tA[0] * Rm[0][j] + tA[1] * Rm[1][j] + tA[2] * Rm[2][j]);
        XMVECTOR axis = (j == 0 ? Bx : (j == 1 ? By : Bz));
        float sep = (ra + rb) - tBj;
        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    // 3) ������ Ai x Bj
    const XMVECTOR Aaxis[3] = { Ax,Ay,Az };
    const XMVECTOR Baxis[3] = { Bx,By,Bz };

    for (int i = 0; i < 3; ++i) for (int j = 0; j < 3; ++j) {
        // ra = a[(i+1)%3]*|R[(i+2)%3][j]| + a[(i+2)%3]*|R[(i+1)%3][j]|
        float ra = (&a.x)[(i + 1) % 3] * AbsR[(i + 2) % 3][j] + (&a.x)[(i + 2) % 3] * AbsR[(i + 1) % 3][j];
        // rb = b[(j+1)%3]*|R[i][(j+2)%3]| + b[(j+2)%3]*|R[i][(j+1)%3]|
        float rb = (&b.x)[(j + 1) % 3] * AbsR[i][(j + 2) % 3] + (&b.x)[(j + 2) % 3] * AbsR[i][(j + 1) % 3];
        // |t �E (Ai �~ Bj)| �� A���ł̎��ɓW�J
        float tProj = std::fabs(tA[(i + 2) % 3] * Rm[(i + 1) % 3][j] - tA[(i + 1) % 3] * Rm[(i + 2) % 3][j]);
        float sep = (ra + rb) - tProj;

        // �@�� = Ai �~ Bj�i���[���h�j
        XMVECTOR axis = XMVector3Cross(Aaxis[i], Baxis[j]);
        // ���s�ɋ߂��ꍇ�̓X�L�b�v
        if (XMVectorGetX(XMVector3LengthSq(axis)) < 1e-12f) continue;

        if (!updateBest(sep, axis, best)) return { false,{0,0,0},0 };
    }

    return best;
}

/**
 * @brief ���C�q�b�g���ʁi�X�C�[�v����p�j
 * - t: 0�`1 �͈̔́i�t���[�����̏Փˎ����j / normal: ���[���h�@��
 */
struct RayHit {
    bool hit = false;
    float t = 1.0f;                 // 0..1
    DirectX::XMFLOAT3 normal{ 0,0,0 }; // world-space
};

/**
 * @brief �_���C vs �u���IOBB�Ŋg�������ÓIOBB(AABB��)�v
 * @param stat �ÓIOBB�i���[���h�j
 * @param dyn  ���IOBB�i���[���h�j���g���ʂ̎Z�o�Ɏg�p
 * @param p0   ���C�J�n�_�i���[���h�j
 * @param v    ���C�����i�t���[���ړ��ʁA���[���h�j
 * @param skin �ڐG���Ɏ�O�֖߂������i��AABB�g���ɂ͎g��Ȃ��j
 */
inline RayHit RaycastPointVsExpandedObb(const WorldObb& stat, const WorldObb& dyn,
    const DirectX::XMFLOAT3& p0,
    const DirectX::XMFLOAT3& v,
    float /*skin*/)
{
    using namespace DirectX;

    // stat��/ dyn��
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

    // �g���� ext = |R| * dyn.half
    XMFLOAT3 extExp = {
        Rabs[0][0] * dyn.half.x + Rabs[0][1] * dyn.half.y + Rabs[0][2] * dyn.half.z,
        Rabs[1][0] * dyn.half.x + Rabs[1][1] * dyn.half.y + Rabs[1][2] * dyn.half.z,
        Rabs[2][0] * dyn.half.x + Rabs[2][1] * dyn.half.y + Rabs[2][2] * dyn.half.z
    };

    // ��skin��AABB���a�ɓ���Ȃ�
    XMFLOAT3 A = {
        stat.half.x + extExp.x,
        stat.half.y + extExp.y,
        stat.half.z + extExp.z
    };

    // ���[���h�� stat���[�J��
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

    // �X���u����
    float tmin = 0.0f, tmax = 1.0f;
    int   hitAxis = -1;
    int   sign = 0;

    auto slab = [&](float p, float d, float a, int axis)->bool {
        const float eps = 1e-8f;
        if (std::fabs(d) < eps) {
            // ���s�F�O�Ȃ��Փ�
            return (std::fabs(p) <= a);
        }
        float inv = 1.0f / d;
        float t1 = (-a - p) * inv;
        float t2 = (a - p) * inv;
        int sgn = (t1 > t2);
        if (sgn) std::swap(t1, t2);

        // �����������e���� "�ǂ����̎��͕K���L�^" �ł���悤�ɂ���
        if (t1 >= tmin) { tmin = t1; hitAxis = axis; sign = sgn ? +1 : -1; }
        if (t2 < tmax) tmax = t2;
        return tmin <= tmax;
        };

    if (!slab(pL.x, vL.x, A.x, 0)) return {};
    if (!slab(pL.y, vL.y, A.y, 1)) return {};
    if (!slab(pL.z, vL.z, A.z, 2)) return {};
    if (tmin < 0.0f || tmin > 1.0f) return {};

    // �����S�K�[�h�F�q�b�g�����m�肵�Ă��Ȃ���΃q�b�g�������i�����X�^�[�g���j
    if (hitAxis < 0 || hitAxis > 2) {
        return {};
    }

    // �@���istat���[�J�������[���h�j
    XMFLOAT3 nLocal{ 0,0,0 };
    (&nLocal.x)[hitAxis] = (sign ? +1.0f : -1.0f);

    // ���K���̑O�Ƀ[���x�N�g���������
    XMVECTOR nW =
        XMVectorScale(XMLoadFloat3(&uS[0]), nLocal.x) +
        XMVectorScale(XMLoadFloat3(&uS[1]), nLocal.y) +
        XMVectorScale(XMLoadFloat3(&uS[2]), nLocal.z);

    // �����`�F�b�N�i�O�̂��߁j
    if (XMVector3Equal(nW, XMVectorZero())) {
        return {};
    }

    nW = XMVector3Normalize(nW);

    RayHit h; h.hit = true; h.t = tmin;
    XMStoreFloat3(&h.normal, nW);
    return h;
}
