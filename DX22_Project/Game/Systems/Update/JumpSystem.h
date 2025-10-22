#pragma once
#include "../../ECS.h"
#include "../IUpdateSystem.h"

class JumpSystem final : public IUpdateSystem {
public:
    JumpSystem(float jumpSpeed = 5.5f, float buffer = 0.10f, float coyote = 0.08f)
        : m_jumpSpeed(jumpSpeed), m_buffer(buffer), m_coyote(coyote) {
    }
    void Update(class World& world, float dt) override;

private:
    float m_jumpSpeed;
    float m_buffer;  // �o�b�t�@���e�b�i�����������Ă�OK�j
    float m_coyote;  // �R���[�e�b�i�������OK�j
};
