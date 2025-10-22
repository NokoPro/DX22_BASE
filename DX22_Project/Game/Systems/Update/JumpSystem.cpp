#include "JumpSystem.h"
#include "../../World.h"
#include "../../Components/Gameplay/CharacterControllerComponent.h"
#include "../../Components/Physics/RigidbodyComponent.h"
#include "../../Components/Physics/GroundingComponent.h"
#include "../../Components/Physics/JumpBufferComponent.h"
#include <cmath>

void JumpSystem::Update(World& world, float dt)
{
    // 1) �o�b�t�@�i�����Ă���̎��ԁj��i�߂�
    world.View<JumpBufferComponent>([&](EntityId, JumpBufferComponent& jb) {
        jb.timeSincePressed += dt;
        });

    // 2) ���Δ���iCollision �� grounded ���m�肵����ɓ����z��j
    world.View<CharacterControllerComponent, RigidbodyComponent,
        GroundingComponent, JumpBufferComponent>(
            [&](EntityId, CharacterControllerComponent& cc, RigidbodyComponent& rb,
                GroundingComponent& gr, JumpBufferComponent& jb)
            {
                const bool buffered = (jb.timeSincePressed <= m_buffer);
                const bool coyoteOK = (gr.grounded || gr.timeSinceGrounded <= m_coyote);

                if (buffered && coyoteOK)
                {
                    // �W�����v����
                    rb.velocity.y = m_jumpSpeed;
                    gr.grounded = false;                // �����󒆂�
                    jb.timeSincePressed = 1e9f;         // ����

                    // ���̓t���O���C�ӂŏ���i�d�����s�̕ی��j
                    cc.jumpPressed = false;
                }
            });
}
