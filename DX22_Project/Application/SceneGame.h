#pragma once
#include "../System/Scene.h"
#include "../System/Input.h"
#include "../System/DirectX/ShaderList.h"
#include "../System/Geometory.h"
#include "../System/Model.h"

#include "../Game/ECS.h"
#include "../Game/World.h"
#include "../Game/Components/TransformComponent.h"
#include "../Game/Components/ModelRendererComponent.h"
#include "../Game/Systems/SystemRegistry.h"
#include "../Game/Systems/Update/OrbitCameraSystem.h"
#include "../Game/Systems/Update/SpinSystem.h"
#include "../Game/Systems/Render/ModelRenderSystem.h"
#include "../Game/Systems/Render/DebugGridRenderSystem.h"
#include <DirectXMath.h>

/**
 * @file SceneGame.h
 * @brief �Q�[���{�҃V�[���iECS�FModel �� Transform+ModelRenderer �ŕ`��j
 */
class SceneGame : public Scene
{
public:
    SceneGame();
    ~SceneGame();

    /** @brief �t���[���X�V�i���́E�J�����E���W�b�N�j */
    void Update() final;

    /** @brief �t���[���`��i���f���E�f�o�b�O�j */
    void Draw() final;

private:
    // === ECS���� ===
    World           m_world;
    SystemRegistry  m_sys;

    // === �J�����iUpdate System�j ===
    OrbitCameraSystem* m_cam = nullptr;

    // === �`��iRender System�Q�Ɓj ===
    ModelRenderSystem* m_drawModel = nullptr;
    bool                m_showGrid = true;

    // === ���Y ===
    Model m_mushroom;

    // === ���͗p�g�O�� ===
    bool m_pauseSpin = false;
};
