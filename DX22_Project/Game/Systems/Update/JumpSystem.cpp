#include "JumpSystem.h"
#include "../../World.h"
#include "../../Components/Gameplay/CharacterControllerComponent.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/GroundingComponent.h"
#include "../../Components/Physics/JumpBufferComponent.h"
#include <cmath>

void JumpSystem::Update(World& world, float dt)
{
    // 1) バッファ（押してからの時間）を進める
    world.View<JumpBufferComponent>([&](EntityId, JumpBufferComponent& jb) {
        jb.timeSincePressed += dt;
        });

    // 2) 発火判定（Collision で grounded が確定した後に動く想定）
    world.View<CharacterControllerComponent, RigidbodyComponent,
        GroundingComponent, JumpBufferComponent>(
            [&](EntityId, CharacterControllerComponent& cc, RigidbodyComponent& rb,
                GroundingComponent& gr, JumpBufferComponent& jb)
            {
                const bool buffered = (jb.timeSincePressed <= m_buffer);
                const bool coyoteOK = (gr.grounded || gr.timeSinceGrounded <= m_coyote);

                if (buffered && coyoteOK)
                {
                    // ジャンプ発火
                    rb.velocity.y = m_jumpSpeed;
                    gr.grounded = false;                // すぐ空中へ
                    jb.timeSincePressed = 1e9f;         // 消費

                    // 入力フラグも任意で消費（重複実行の保険）
                    cc.jumpPressed = false;
                }
            });
}
