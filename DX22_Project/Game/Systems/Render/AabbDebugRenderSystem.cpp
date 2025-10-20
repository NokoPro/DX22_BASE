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
    XMStoreFloat4x4(&Wt, XMMatrixTranspose(W)); // Geometoryは転置行列を想定
    Geometory::SetWorld(Wt);

    // --- ここで半透明の薄い緑にしたい ---
    const DirectX::XMFLOAT4 fill = { 0.6f, 1.0f, 0.6f, 0.25f };
    Geometory::SetColor(fill);
    Geometory::DrawBox(); // ← 既存呼び出し
}

static inline void AddAabbWire(
    const XMFLOAT3& min, const XMFLOAT3& max,
    const XMFLOAT4& color /* 線の色（不透明推奨） */)
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
    // 表示OFFなら描かない
    if (m_enabled && !*m_enabled) return;

    // 線の色：薄い青（輪郭は不透明で見やすく）
    const XMFLOAT4 wireCol{ 0.6f, 0.6f, 1.0f, 1.0f };

    // 全AABBを走査して「半透明の箱＋緑ライン」を描画
    world.View<TransformComponent, AabbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const AabbColliderComponent& col)
        {
            // 中心と半径
            const XMFLOAT3 center{
                tr.position.x + col.offset.x,
                tr.position.y + col.offset.y,
                tr.position.z + col.offset.z
            };
            const XMFLOAT3 half = col.halfExtents;

            // 1) 半透明の箱（薄い緑）を描画
            DrawBoxWorld(center, half);

            // 2) 輪郭ライン（濃い緑）を重ねる
            const XMFLOAT3 mn{ center.x - half.x, center.y - half.y, center.z - half.z };
            const XMFLOAT3 mx{ center.x + half.x, center.y + half.y, center.z + half.z };
            AddAabbWire(mn, mx, wireCol);
        });

    Geometory::DrawLines(); // 蓄積したラインをまとめて描画
}

