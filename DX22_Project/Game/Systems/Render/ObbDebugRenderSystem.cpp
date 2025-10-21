#include "ObbDebugRenderSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Physics/ObbColliderComponent.h"
#include "../../Systems/Physics/ObbUtil.h"
#include "../../../System/Geometory.h"
#include <DirectXMath.h>

using namespace DirectX;

// 追加：AABB版の DrawBoxWorld に相当する OBB 版
static inline void DrawObbWorld(const WorldObb& w, const DirectX::XMFLOAT4& rgba)
{
    using namespace DirectX;

    // 列（col） = 軸ベクトル × 2*half
    const XMFLOAT3 cx{ w.axis[0].x * (w.half.x * 2.0f),
                       w.axis[0].y * (w.half.x * 2.0f),
                       w.axis[0].z * (w.half.x * 2.0f) };
    const XMFLOAT3 cy{ w.axis[1].x * (w.half.y * 2.0f),
                       w.axis[1].y * (w.half.y * 2.0f),
                       w.axis[1].z * (w.half.y * 2.0f) };
    const XMFLOAT3 cz{ w.axis[2].x * (w.half.z * 2.0f),
                       w.axis[2].y * (w.half.z * 2.0f),
                       w.axis[2].z * (w.half.z * 2.0f) };

    // 転置済みW行列（Wt）を直接構築
    //  [ cx.x  cy.x  cz.x  center.x ]
    //  [ cx.y  cy.y  cz.y  center.y ]
    //  [ cx.z  cy.z  cz.z  center.z ]
    //  [  0      0     0       1    ]
    XMFLOAT4X4 Wt;
    Wt._11 = cx.x; Wt._12 = cy.x; Wt._13 = cz.x; Wt._14 = w.center.x;
    Wt._21 = cx.y; Wt._22 = cy.y; Wt._23 = cz.y; Wt._24 = w.center.y;
    Wt._31 = cx.z; Wt._32 = cy.z; Wt._33 = cz.z; Wt._34 = w.center.z;
    Wt._41 = 0.0f; Wt._42 = 0.0f; Wt._43 = 0.0f; Wt._44 = 1.0f;

    Geometory::SetWorld(Wt);
    Geometory::SetColor(rgba);
    Geometory::DrawBox();
}


static inline void AddObbWire(const WorldObb& w, const XMFLOAT4& color)
{
    // 8頂点を作成：center ± axis[i]*half[i]
    auto vtx = [&](float sx, float sy, float sz)->XMFLOAT3 {
        XMFLOAT3 p = w.center;
        p.x += w.axis[0].x * w.half.x * sx + w.axis[1].x * w.half.y * sy + w.axis[2].x * w.half.z * sz;
        p.y += w.axis[0].y * w.half.x * sx + w.axis[1].y * w.half.y * sy + w.axis[2].y * w.half.z * sz;
        p.z += w.axis[0].z * w.half.x * sx + w.axis[1].z * w.half.y * sy + w.axis[2].z * w.half.z * sz;
        return p;
        };

    const XMFLOAT3 p000 = vtx(-1, -1, -1);
    const XMFLOAT3 p001 = vtx(-1, -1, +1);
    const XMFLOAT3 p010 = vtx(-1, +1, -1);
    const XMFLOAT3 p011 = vtx(-1, +1, +1);
    const XMFLOAT3 p100 = vtx(+1, -1, -1);
    const XMFLOAT3 p101 = vtx(+1, -1, +1);
    const XMFLOAT3 p110 = vtx(+1, +1, -1);
    const XMFLOAT3 p111 = vtx(+1, +1, +1);

    // 12 edges
    Geometory::AddLine(p000, p001, color); Geometory::AddLine(p001, p011, color);
    Geometory::AddLine(p011, p010, color); Geometory::AddLine(p010, p000, color);

    Geometory::AddLine(p100, p101, color); Geometory::AddLine(p101, p111, color);
    Geometory::AddLine(p111, p110, color); Geometory::AddLine(p110, p100, color);

    Geometory::AddLine(p000, p100, color); Geometory::AddLine(p001, p101, color);
    Geometory::AddLine(p011, p111, color); Geometory::AddLine(p010, p110, color);
}

void ObbDebugRenderSystem::Render(const World& world)
{
    if (m_enabled && !*m_enabled) return;

    const DirectX::XMFLOAT4 fill{ 0.6f, 1.0f, 0.6f, 0.25f };
    const DirectX::XMFLOAT4 wire{ 0.2f, 0.8f, 0.2f, 1.0f };

    world.View<TransformComponent, ObbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const ObbColliderComponent& obb)
        {
            const WorldObb w = MakeWorldObb(tr, obb);

            // 1) 面（半透明）
            DrawObbWorld(w, fill);

            // 2) 輪郭ライン
            AddObbWire(w, wire);
        });

    Geometory::DrawLines();
}
