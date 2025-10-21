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
#include "../Game/Systems/Update/FollowCameraSystem.h"
#include "../Game/Systems/Update/SpinSystem.h"
#include "../Game/Systems/Render/ModelRenderSystem.h"
#include "../Game/Systems/Render/DebugGridRenderSystem.h"
#include "../Game/Systems/Render/AabbDebugRenderSystem.h"

#include "../Game/Systems/Update/PlayerInputSystem.h"
#include "../Game/Systems/Update/MovementControlSystem.h"
#include "../Game/Systems/Update/PhysicsSystem.h"
#include "../Game/Systems/Update/CollisionSystem.h"

#include "../Game/Prefabs/PrefabRegistry.h"

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
    FollowCameraSystem* m_cam = nullptr;

    // === �`��iRender System�Q�Ɓj ===
    ModelRenderSystem* m_drawModel = nullptr;
    bool                m_showGrid = true;
    bool                m_showColliders = false; // F2��ON/OFF

    // === ���Y ===
    Model m_mushroom;
    Model m_ground;

    // === ECS �G���e�B�e�BID�ێ� ===
    EntityId m_playerEntity = 0; // <- �v���C���[�G���e�B�e�BID��ێ����邽�߂ɒǉ�

    // === ���͗p�g�O�� ===
    bool m_pauseSpin = false;

    PrefabRegistry m_prefabs;
};
