#include "AabbDebugRenderSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Physics/AabbColliderComponent.h"
#include "../../../System/Geometory.h"
#include <DirectXMath.h>

using namespace DirectX;

static inline void DrawBoxWorld(const XMFLOAT3& center, const XMFLOAT3& half)
{
    const XMMATRIX S = XMMatrixScaling(half.x * 2.0f, half.y * 2.0f, half.z * 2.0f);
    const XMMATRIX T = XMMatrixTranslation(center.x, center.y, center.z);
    const XMMATRIX W = S * T;

    XMFLOAT4X4 Wt;
    XMStoreFloat4x4(&Wt, XMMatrixTranspose(W)); // Geometory�͓]�u�s���z��
    Geometory::SetWorld(Wt);

    // --- �����Ŕ������̔����΂ɂ����� ---
    const DirectX::XMFLOAT4 fill = { 0.6f, 1.0f, 0.6f, 0.25f };
    Geometory::SetColor(fill);
    Geometory::DrawBox(); // �� �����Ăяo��
}

static inline void AddAabbWire(
    const XMFLOAT3& min, const XMFLOAT3& max,
    const XMFLOAT4& color /* ���̐F�i�s���������j */)
{
    const XMFLOAT3 p000{ min.x, min.y, min.z };
    const XMFLOAT3 p001{ min.x, min.y, max.z };
    const XMFLOAT3 p010{ min.x, max.y, min.z };
    const XMFLOAT3 p011{ min.x, max.y, max.z };
    const XMFLOAT3 p100{ max.x, min.y, min.z };
    const XMFLOAT3 p101{ max.x, min.y, max.z };
    const XMFLOAT3 p110{ max.x, max.y, min.z };
    const XMFLOAT3 p111{ max.x, max.y, max.z };

    // 12 edges
    Geometory::AddLine(p000, p001, color); Geometory::AddLine(p001, p011, color);
    Geometory::AddLine(p011, p010, color); Geometory::AddLine(p010, p000, color);

    Geometory::AddLine(p100, p101, color); Geometory::AddLine(p101, p111, color);
    Geometory::AddLine(p111, p110, color); Geometory::AddLine(p110, p100, color);

    Geometory::AddLine(p000, p100, color); Geometory::AddLine(p001, p101, color);
    Geometory::AddLine(p011, p111, color); Geometory::AddLine(p010, p110, color);
}

void AabbDebugRenderSystem::Render(const World& world)
{
    // �\��OFF�Ȃ�`���Ȃ�
    if (m_enabled && !*m_enabled) return;

    // ���̐F�F�����i�֊s�͕s�����Ō��₷���j
    const XMFLOAT4 wireCol{ 0.6f, 0.6f, 1.0f, 1.0f };

    // �SAABB�𑖍����āu�������̔��{�΃��C���v��`��
    world.View<TransformComponent, AabbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const AabbColliderComponent& col)
        {
            // ���S�Ɣ��a
            const XMFLOAT3 center{
                tr.position.x + col.offset.x,
                tr.position.y + col.offset.y,
                tr.position.z + col.offset.z
            };
            const XMFLOAT3 half = col.halfExtents;

            // 1) �������̔��i�����΁j��`��
            DrawBoxWorld(center, half);

            // 2) �֊s���C���i�Z���΁j���d�˂�
            const XMFLOAT3 mn{ center.x - half.x, center.y - half.y, center.z - half.z };
            const XMFLOAT3 mx{ center.x + half.x, center.y + half.y, center.z + half.z };
            AddAabbWire(mn, mx, wireCol);
        });

    Geometory::DrawLines(); // �~�ς������C�����܂Ƃ߂ĕ`��
}

