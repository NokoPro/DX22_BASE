#include "DebugGridRenderSystem.h"
#include "../../../System/Geometory.h"
#include <DirectXMath.h>
using namespace DirectX;

void DebugGridRenderSystem::Render(const World&)  // ← シグネチャを基底と一致させる
{
    if (!m_flag || !*m_flag) return;

    const XMFLOAT4 gridCol(0.5f, 0.5f, 0.5f, 1);
    const float size = 20.f, step = 1.f;

    for (float g = -size; g <= size; g += step)
    {
        Geometory::AddLine({ g, 0, -size }, { g, 0,  size }, gridCol);
        Geometory::AddLine({ -size, 0, g }, { size, 0, g }, gridCol);
    }
    Geometory::AddLine({ 0,0,0 }, { 3,0,0 }, { 1,0,0,1 });
    Geometory::AddLine({ 0,0,0 }, { 0,3,0 }, { 0,1,0,1 });
    Geometory::AddLine({ 0,0,0 }, { 0,0,3 }, { 0,0,1,1 });
    Geometory::DrawLines();
}
