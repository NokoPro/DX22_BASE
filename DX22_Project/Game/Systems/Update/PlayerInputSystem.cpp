#include "PlayerInputSystem.h"
#include "../../World.h"
#include "../../Components/Gameplay/CharacterControllerComponent.h"

// �t���[�����[�N�̃L�[�{�[�h����
#include "../../../System/Input.h"

// XInput�i�Q�[���p�b�h�j
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")  // ���ɉ����� xinput.lib �ł�OK

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>

using std::max;
using std::min;

// --- �ǉ��F�ȈՃL�[�{�[�h�w���p�i�t���[������/�����オ��j ---
static inline bool KeyHeld(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}
static inline bool KeyPressed(int vk) {
    // ����: TU���ÓI�o�b�t�@�ŊȈ՗����オ�茟�o�i�v���Z�X���ŗL���j
    static SHORT prev[256] = { 0 };
    SHORT s = GetAsyncKeyState(vk);
    bool now = (s & 0x8000) != 0;
    bool was = (prev[vk] & 0x8000) != 0;
    prev[vk] = s;
    return (now && !was);
}


/**
 * @brief �������[�e�B���e�B�F�x�N�g�����K���i2D�j
 */
static inline void normalize2(float& x, float& z)
{
    const float len = std::sqrt(x * x + z * z);
    if (len > 1e-6f) { x /= len; z /= len; }
}

/**
 * @brief �������[�e�B���e�B�F�f�b�h�]�[���K�p�i-1..+1 �͈́j
 */
static inline void applyDeadZone(float& x, float& z, float deadZone)
{
    const float len = std::sqrt(x * x + z * z);
    if (len < deadZone) { x = 0.f; z = 0.f; return; }
    // �f�b�h�]�[���O���� 0..1 �ɍă}�b�v�i�X�e�B�b�N���x�����炩�Ɂj
    const float k = (len - deadZone) / (1.f - deadZone);
    if (k > 0.f) { x = (x / len) * k; z = (z / len) * k; }
    else { x = z = 0.f; }
}

/**
 * @brief XInput: -32768..32767 �� -1..+1 �ɕϊ�
 */
static inline float axisToUnit(short v)
{
    // NOTE: 32767�Ŋ���� -32768 ���ŋ͂��Ȕ�Ώ̂��o�邪���p�㋖�e
    return (v >= 0) ? (static_cast<float>(v) / 32767.0f)
        : (static_cast<float>(v) / 32768.0f);
}

void PlayerInputSystem::Update(World& world, float /*dt*/)
{
    // =======================
    // 1) ���͂̎��W�iKeyboard�j
    // =======================
    float kbX = 0.f, kbZ = 0.f;
    if (KeyHeld('A')) kbX -= 1.f;
    if (KeyHeld('D')) kbX += 1.f;
    if (KeyHeld('W')) kbZ += 1.f;
    if (KeyHeld('S')) kbZ -= 1.f;
    const bool kbRun = (KeyHeld(VK_LSHIFT) || KeyHeld(VK_RSHIFT));
    const bool kbJump = KeyPressed(VK_SPACE);
    // �΂߉����h�~
    if (kbX != 0.f || kbZ != 0.f) normalize2(kbX, kbZ);

    // =======================
    // 2) ���͂̎��W�iXInput�j
    // =======================
    static DWORD s_prevButtons[4] = { 0,0,0,0 };  // �O�t���[���̃{�^���i�����オ�茟�o�p�j
    float padX = 0.f, padZ = 0.f;
    bool  padRun = false;
    bool  padJumpPressed = false;

    // �ŏ��Ɍ��������ڑ��p�b�h���g���i�K�v�Ȃ�1P�Œ�ȂǂɊg���j
    for (DWORD padIndex = 0; padIndex < 4; ++padIndex)
    {
        XINPUT_STATE st;
        std::memset(&st, 0, sizeof(st));
        if (XInputGetState(padIndex, &st) != ERROR_SUCCESS) continue;

        // ���X�e�B�b�N
        padX = axisToUnit(st.Gamepad.sThumbLX);
        padZ = axisToUnit(st.Gamepad.sThumbLY); // ��� +Z �Ƃ��Ĉ���

        // �f�b�h�]�[���K�p �� ���K�����N�����v
        applyDeadZone(padX, padZ, std::max(0.f, std::min(m_deadZone, 0.99f)));
        if (padX != 0.f || padZ != 0.f)
        {
            const float len = std::sqrt(padX * padX + padZ * padZ);
            if (len > 1.f) { padX /= len; padZ /= len; }
        }

        // �_�b�V���i�C�ӁF���E�V�����_�[ or ���X�e�B�b�N�������݁j
        const WORD b = st.Gamepad.wButtons;
        padRun = ((b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0) ||
            ((b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0) ||
            ((b & XINPUT_GAMEPAD_LEFT_THUMB) != 0);

        // �W�����v��A�{�^���̗����オ��
        const bool aNow = (b & XINPUT_GAMEPAD_A) != 0;
        const bool aPrev = (s_prevButtons[padIndex] & XINPUT_GAMEPAD_A) != 0;
        padJumpPressed = padJumpPressed || (aNow && !aPrev); // �ǂꂩ�̃p�b�h�ŗ����オ���OK

        s_prevButtons[padIndex] = b;
        break; // 1��ڂŊm��i���������������ꍇ�� break ���O���j
    }

    // =======================
    // 3) ���͂̓����iKeyboard + Pad�j
    // =======================
    float x = kbX + padX;
    float z = kbZ + padZ;
    // ��������΂߉����h�~
    if (x != 0.f || z != 0.f) {
        const float len = std::sqrt(x * x + z * z);
        if (len > 1.f) { x /= len; z /= len; }
    }

    const bool runModifier = (kbRun || padRun);
    const bool jumpPressed = (kbJump || padJumpPressed);

    // =======================
    // 4) World �֔��f�i�Ӑ}�����j
    // =======================
    world.View<CharacterControllerComponent>([&](EntityId, CharacterControllerComponent& cc)
        {
            cc.moveInput = { x, z };
            cc.runModifier = runModifier;
            cc.jumpPressed = jumpPressed;
        });
}
