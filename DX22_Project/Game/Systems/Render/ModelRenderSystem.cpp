#include "ModelRenderSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"
#include "../../Components/ModelRendererComponent.h"
#include "../../../System/DirectX/ShaderList.h"
#include "../../../System/Model.h"
#include <DirectXMath.h>

using namespace DirectX;

void ModelRenderSystem::Render(const World& world)
{
    using namespace DirectX;

    world.View<TransformComponent, ModelRendererComponent>(
        [&](EntityId, const TransformComponent& tr, const ModelRendererComponent& mr)
        {
            if (!mr.visible || !mr.model) return;

            const XMMATRIX S = XMMatrixScaling(tr.scale.x, tr.scale.y, tr.scale.z);
            const XMMATRIX Rx = XMMatrixRotationX(XMConvertToRadians(tr.rotationDeg.x));
            const XMMATRIX Ry = XMMatrixRotationY(XMConvertToRadians(tr.rotationDeg.y));
            const XMMATRIX Rz = XMMatrixRotationZ(XMConvertToRadians(tr.rotationDeg.z));
            const XMMATRIX T = XMMatrixTranslation(tr.position.x, tr.position.y, tr.position.z);
            const XMMATRIX W = S * Rx * Ry * Rz * T;

            XMFLOAT4X4 wvp[3];
            XMStoreFloat4x4(&wvp[0], XMMatrixTranspose(W));
            wvp[1] = m_V;
            wvp[2] = m_P;

            ShaderList::SetWVP(wvp);
            mr.model->Draw();
        });
}

void ModelRenderSystem::ApplyDefaultLighting(float camY, float camRadius)
{
    ShaderList::SetLight({ 1,1,1,1 }, { -1,-1,-1 });
    ShaderList::SetCameraPos({ 0.0f, camY, -camRadius });
}
