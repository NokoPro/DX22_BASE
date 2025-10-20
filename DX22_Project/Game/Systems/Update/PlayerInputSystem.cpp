#include "PlayerInputSystem.h"
#include "../../World.h"
#include "../../Components/Gameplay/CharacterControllerComponent.h"

// フレームワークのキーボード入力
#include "../../../System/Input.h"

// XInput（ゲームパッド）
#include <Xinput.h>
#pragma comment(lib, "xinput9_1_0.lib")  // 環境に応じて xinput.lib でもOK

#include <Windows.h>
#include <algorithm>
#include <cmath>
#include <cstring>

using std::max;
using std::min;

// --- 追加：簡易キーボードヘルパ（フレーム持続/立ち上がり） ---
static inline bool KeyHeld(int vk) {
    return (GetAsyncKeyState(vk) & 0x8000) != 0;
}
static inline bool KeyPressed(int vk) {
    // 注意: TU内静的バッファで簡易立ち上がり検出（プロセス内で有効）
    static SHORT prev[256] = { 0 };
    SHORT s = GetAsyncKeyState(vk);
    bool now = (s & 0x8000) != 0;
    bool was = (prev[vk] & 0x8000) != 0;
    prev[vk] = s;
    return (now && !was);
}


/**
 * @brief 内部ユーティリティ：ベクトル正規化（2D）
 */
static inline void normalize2(float& x, float& z)
{
    const float len = std::sqrt(x * x + z * z);
    if (len > 1e-6f) { x /= len; z /= len; }
}

/**
 * @brief 内部ユーティリティ：デッドゾーン適用（-1..+1 範囲）
 */
static inline void applyDeadZone(float& x, float& z, float deadZone)
{
    const float len = std::sqrt(x * x + z * z);
    if (len < deadZone) { x = 0.f; z = 0.f; return; }
    // デッドゾーン外側を 0..1 に再マップ（スティック感度を滑らかに）
    const float k = (len - deadZone) / (1.f - deadZone);
    if (k > 0.f) { x = (x / len) * k; z = (z / len) * k; }
    else { x = z = 0.f; }
}

/**
 * @brief XInput: -32768..32767 を -1..+1 に変換
 */
static inline float axisToUnit(short v)
{
    // NOTE: 32767で割ると -32768 側で僅かな非対称が出るが実用上許容
    return (v >= 0) ? (static_cast<float>(v) / 32767.0f)
        : (static_cast<float>(v) / 32768.0f);
}

void PlayerInputSystem::Update(World& world, float /*dt*/)
{
    // =======================
    // 1) 入力の収集（Keyboard）
    // =======================
    float kbX = 0.f, kbZ = 0.f;
    if (KeyHeld('A')) kbX -= 1.f;
    if (KeyHeld('D')) kbX += 1.f;
    if (KeyHeld('W')) kbZ += 1.f;
    if (KeyHeld('S')) kbZ -= 1.f;
    const bool kbRun = (KeyHeld(VK_LSHIFT) || KeyHeld(VK_RSHIFT));
    const bool kbJump = KeyPressed(VK_SPACE);
    // 斜め加速防止
    if (kbX != 0.f || kbZ != 0.f) normalize2(kbX, kbZ);

    // =======================
    // 2) 入力の収集（XInput）
    // =======================
    static DWORD s_prevButtons[4] = { 0,0,0,0 };  // 前フレームのボタン（立ち上がり検出用）
    float padX = 0.f, padZ = 0.f;
    bool  padRun = false;
    bool  padJumpPressed = false;

    // 最初に見つかった接続パッドを使う（必要なら1P固定などに拡張）
    for (DWORD padIndex = 0; padIndex < 4; ++padIndex)
    {
        XINPUT_STATE st;
        std::memset(&st, 0, sizeof(st));
        if (XInputGetState(padIndex, &st) != ERROR_SUCCESS) continue;

        // 左スティック
        padX = axisToUnit(st.Gamepad.sThumbLX);
        padZ = axisToUnit(st.Gamepad.sThumbLY); // 上を +Z として扱う

        // デッドゾーン適用 → 正規化＆クランプ
        applyDeadZone(padX, padZ, std::max(0.f, std::min(m_deadZone, 0.99f)));
        if (padX != 0.f || padZ != 0.f)
        {
            const float len = std::sqrt(padX * padX + padZ * padZ);
            if (len > 1.f) { padX /= len; padZ /= len; }
        }

        // ダッシュ（任意：左右ショルダー or 左スティック押し込み）
        const WORD b = st.Gamepad.wButtons;
        padRun = ((b & XINPUT_GAMEPAD_LEFT_SHOULDER) != 0) ||
            ((b & XINPUT_GAMEPAD_RIGHT_SHOULDER) != 0) ||
            ((b & XINPUT_GAMEPAD_LEFT_THUMB) != 0);

        // ジャンプ＝Aボタンの立ち上がり
        const bool aNow = (b & XINPUT_GAMEPAD_A) != 0;
        const bool aPrev = (s_prevButtons[padIndex] & XINPUT_GAMEPAD_A) != 0;
        padJumpPressed = padJumpPressed || (aNow && !aPrev); // どれかのパッドで立ち上がればOK

        s_prevButtons[padIndex] = b;
        break; // 1台目で確定（複数合成したい場合は break を外す）
    }

    // =======================
    // 3) 入力の統合（Keyboard + Pad）
    // =======================
    float x = kbX + padX;
    float z = kbZ + padZ;
    // 合成後も斜め加速防止
    if (x != 0.f || z != 0.f) {
        const float len = std::sqrt(x * x + z * z);
        if (len > 1.f) { x /= len; z /= len; }
    }

    const bool runModifier = (kbRun || padRun);
    const bool jumpPressed = (kbJump || padJumpPressed);

    // =======================
    // 4) World へ反映（意図だけ）
    // =======================
    world.View<CharacterControllerComponent>([&](EntityId, CharacterControllerComponent& cc)
        {
            cc.moveInput = { x, z };
            cc.runModifier = runModifier;
            cc.jumpPressed = jumpPressed;
        });
}
