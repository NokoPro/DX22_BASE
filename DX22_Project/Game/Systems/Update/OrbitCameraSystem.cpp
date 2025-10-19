#include "OrbitCameraSystem.h"
#include <DirectXMath.h>
#include <cmath>
#include "../../../System/Geometory.h"

using namespace DirectX;

OrbitCameraSystem::OrbitCameraSystem(float radius, float height,
    float fovYDeg, float aspect,
    float zNear, float zFar)
    : m_radius(radius), m_height(height),
    m_fovYDeg(fovYDeg), m_aspect(aspect),
    m_zNear(zNear), m_zFar(zFar) {
}

void OrbitCameraSystem::Update(World&, float dt) 
{
    m_angle += 0.4f * dt;

    const float cx = std::cos(m_angle) * m_radius;
    const float cz = std::sin(m_angle) * m_radius;

    const XMVECTOR eye = XMVectorSet(cx, m_height, cz, 0);
    const XMVECTOR at = XMVectorSet(0, 0.8f, 0, 0);
    const XMVECTOR up = XMVectorSet(0, 1, 0, 0);

    const XMMATRIX V = XMMatrixLookAtLH(eye, at, up);
    const XMMATRIX P = XMMatrixPerspectiveFovLH(
        XMConvertToRadians(m_fovYDeg), m_aspect, m_zNear, m_zFar);

    XMStoreFloat4x4(&m_V, XMMatrixTranspose(V));
    XMStoreFloat4x4(&m_P, XMMatrixTranspose(P));

    // デバッグライン描画用カメラも同期
    Geometory::SetView(m_V);
    Geometory::SetProjection(m_P);
}
