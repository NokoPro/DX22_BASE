#include "ObbDebugRenderSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/Physics/ObbColliderComponent.h"
#include "../../Systems/Physics/ObbUtil.h"
#include "../../../System/Geometory.h"
#include <DirectXMath.h>

using namespace DirectX;

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

    const XMFLOAT4 fill{ 0.6f, 1.0f, 0.6f, 0.25f }; // 半透明の薄い緑
    const XMFLOAT4 wire{ 0.2f, 0.8f, 0.2f, 1.0f };  // 濃いめの緑ライン

    world.View<TransformComponent, ObbColliderComponent>(
        [&](EntityId, const TransformComponent& tr, const ObbColliderComponent& obb)
        {
            // 1) WorldObb を作成
            const WorldObb w = MakeWorldObb(tr, obb);

            // 2) ワールド行列(転置)を作って半透明で塗る
            const XMFLOAT4X4 Wt = MakeWorldMatrixT(w);
            Geometory::SetWorld(Wt);
            Geometory::SetColor(fill);
            Geometory::DrawBox();

            // 3) 輪郭線を重ねる
            AddObbWire(w, wire);
        });

    Geometory::DrawLines();
}
