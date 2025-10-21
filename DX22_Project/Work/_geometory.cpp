#include "../System/Geometory.h"
#include <vector>
#include <cmath>

static inline Geometory::Vertex MakeV(float x, float y, float z, float u, float v)
{
    Geometory::Vertex out{};
    out.pos[0] = x; out.pos[1] = y; out.pos[2] = z;
    out.uv[0] = u; out.uv[1] = v;
    return out;
}

void Geometory::MakeBox()
{
    // �P�ʃ{�b�N�X�F���S���_�A�e�Ӓ� 1�i�}0.5�j
    constexpr float s = 0.5f;

    // �ʂ��Ƃ�UV��\�邽�� 24���_�i6�ʁ~4�j
    Vertex vtx[] = {
        // +Z (front)
        MakeV(-s,-s, s, 0,1), MakeV(s,-s, s, 1,1),
        MakeV(-s, s, s, 0,0), MakeV(s, s, s, 1,0),
        // -Z (back)
        MakeV(s,-s,-s, 0,1), MakeV(-s,-s,-s, 1,1),
        MakeV(s, s,-s, 0,0), MakeV(-s, s,-s, 1,0),
        // +X (right)
        MakeV(s,-s, s, 0,1), MakeV(s,-s,-s, 1,1),
        MakeV(s, s, s, 0,0), MakeV(s, s,-s, 1,0),
        // -X (left)
        MakeV(-s,-s,-s, 0,1), MakeV(-s,-s, s, 1,1),
        MakeV(-s, s,-s, 0,0), MakeV(-s, s, s, 1,0),
        // +Y (top)
        MakeV(-s, s, s, 0,1), MakeV(s, s, s, 1,1),
        MakeV(-s, s,-s, 0,0), MakeV(s, s,-s, 1,0),
        // -Y (bottom)
        MakeV(-s,-s,-s, 0,1), MakeV(s,-s,-s, 1,1),
        MakeV(-s,-s, s, 0,0), MakeV(s,-s, s, 1,0),
    };

    // �e�� 2�O�p�`
    uint32_t idx[] = {
        0,1,2, 2,1,3,      4,5,6, 6,5,7,
        8,9,10, 10,9,11,   12,13,14, 14,13,15,
        16,17,18, 18,17,19, 20,21,22, 22,21,23
    };

    MeshBuffer::Description desc{};
    desc.pVtx = vtx;
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)_countof(vtx);
    desc.pIdx = idx;
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)_countof(idx);
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pBox = new MeshBuffer();
    m_pBox->Create(desc);
}

void Geometory::MakeCylinder()
{
    // ���a0.5, ����1.0�iy: -0.5..+0.5�j
    const int N = CIRCLE_DETAIL;
    const float r = 0.5f;
    const float yTop = 0.5f;
    const float yBot = -0.5f;

    std::vector<Vertex> vtx;
    std::vector<uint32_t> idx;

    // �V�ʒ��S�E��ʒ��S
    uint32_t topCenter = (uint32_t)vtx.size();
    vtx.push_back(MakeV(0, yTop, 0, 0.5f, 0.5f));
    uint32_t botCenter = (uint32_t)vtx.size();
    vtx.push_back(MakeV(0, yBot, 0, 0.5f, 0.5f));

    // �V/�ꃊ���O���_�iUV�͊ȈՂŉ~��[0..1]�֓��e�j
    std::vector<uint32_t> topRing, botRing;
    topRing.reserve(N);
    botRing.reserve(N);

    for (int i = 0; i < N; ++i) {
        float t = (float)i / N;
        float ang = t * 2.0f * 3.1415926535f;
        float cx = std::cos(ang), sz = std::sin(ang);

        // �V��
        topRing.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yTop, sz * r, 0.5f + cx * 0.5f, 0.5f - sz * 0.5f));
        // ���
        botRing.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yBot, sz * r, 0.5f + cx * 0.5f, 0.5f + sz * 0.5f)); // ���ʂɂȂ�̂�V�𔽓]�C����
    }

    // ���ʃ����O�iu: ������, v: �����j
    std::vector<uint32_t> sideTop, sideBot;
    sideTop.reserve(N + 1);
    sideBot.reserve(N + 1);
    for (int i = 0; i <= N; ++i) { // �V�[������邽�� N==0 �Ɠ����_�𖖔���1�ǉ�
        int k = i % N;
        float t = (float)k / N;
        float ang = t * 2.0f * 3.1415926535f;
        float cx = std::cos(ang), sz = std::sin(ang);
        sideTop.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yTop, sz * r, t, 0.0f));
        sideBot.push_back((uint32_t)vtx.size());
        vtx.push_back(MakeV(cx * r, yBot, sz * r, t, 1.0f));
    }

    // --- �C���f�b�N�X
    // --- �V�ʁi+Y �@���j�B�ォ�猩�� CW �ɂȂ�悤 center, next, curr
    for (int i = 0; i < N; ++i) {
        uint32_t a = topCenter;
        uint32_t b = topRing[(i + 1) % N];
        uint32_t c = topRing[i];
        idx.insert(idx.end(), { a,b,c });
    }

    // --- ��ʁi-Y �@���j�B�����猩�� CW�i= �ォ�猩��� CCW�j�ɂȂ�悤 center, curr, next
    for (int i = 0; i < N; ++i) {
        uint32_t a = botCenter;
        uint32_t b = botRing[i];
        uint32_t c = botRing[(i + 1) % N];
        idx.insert(idx.end(), { a,b,c });
    }

    // --- ���ʁi�O�����j�B�c�X�g���b�v�� 2 �O�p�`�F (t0, t1, b0), (t1, b1, b0)
    for (int i = 0; i < N; ++i) {
        uint32_t t0 = sideTop[i];
        uint32_t b0 = sideBot[i];
        uint32_t t1 = sideTop[i + 1];
        uint32_t b1 = sideBot[i + 1];
        idx.insert(idx.end(), { t0, t1, b0,   t1, b1, b0 });
    }

    MeshBuffer::Description desc{};
    desc.pVtx = vtx.data();
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)vtx.size();
    desc.pIdx = idx.data();
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)idx.size();
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pCylinder = new MeshBuffer();
    m_pCylinder->Create(desc);
}

void Geometory::MakeSphere()
{
    // �o�x(slices)�~�ܓx(stacks) ��UV��
    const int stacks = CIRCLE_DETAIL;        // �ܓx�i��k�j
    const int slices = CIRCLE_DETAIL * 2;    // �o�x�i�����j
    const float r = 0.5f;

    std::vector<Vertex> vtx;
    std::vector<uint32_t> idx;
    vtx.reserve((stacks + 1) * (slices + 1));

    for (int y = 0; y <= stacks; ++y) {
        float v = (float)y / stacks;
        float phi = v * 3.1415926535f;            // 0..��
        float cp = std::cos(phi), sp = std::sin(phi);
        for (int x = 0; x <= slices; ++x) {
            float u = (float)x / slices;
            float theta = u * 2.0f * 3.1415926535f; // 0..2��
            float ct = std::cos(theta), st = std::sin(theta);
            float px = r * sp * ct;
            float py = r * cp;
            float pz = r * sp * st;
            vtx.push_back(MakeV(px, py, pz, u, 1.0f - v));
        }
    }

    // 2�O�p�`/�N�A�b�h
    auto Id = [slices](int yy, int xx) { return (uint32_t)(yy * (slices + 1) + xx); };
    for (int y = 0; y < stacks; ++y) {
        for (int x = 0; x < slices; ++x) {
            uint32_t i0 = Id(y, x);
            uint32_t i1 = Id(y, x + 1);
            uint32_t i2 = Id(y + 1, x);
            uint32_t i3 = Id(y + 1, x + 1);
            idx.insert(idx.end(), { i0,i2,i1,  i1,i2,i3 });
        }
    }

    MeshBuffer::Description desc{};
    desc.pVtx = vtx.data();
    desc.vtxSize = sizeof(Vertex);
    desc.vtxCount = (UINT)vtx.size();
    desc.pIdx = idx.data();
    desc.idxSize = sizeof(uint32_t);
    desc.idxCount = (UINT)idx.size();
    desc.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    m_pSphere = new MeshBuffer();
    m_pSphere->Create(desc);
}