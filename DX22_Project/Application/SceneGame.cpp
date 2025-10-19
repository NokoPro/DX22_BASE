#include "SceneGame.h"
#include <cmath>

/** @brief 角度の毎秒回転速度（ラジアン） */
static constexpr float kCamAngularSpeed = 0.4f;
/** @brief カメラ半径 */
static constexpr float kCamRadius = 5.0f;
/** @brief カメラ高さ */
static constexpr float kCamHeight = 3.0f;
/** @brief 垂直視野角（度） */
static constexpr float kFovYDeg = 60.0f;
/** @brief 近/遠クリップ */
static constexpr float kZNear = 0.1f, kZFar = 1000.0f;

SceneGame::SceneGame()
{
    // 行列初期化
    for (int i = 0; i < 3; ++i)
        DirectX::XMStoreFloat4x4(&m_wvp[i], DirectX::XMMatrixIdentity());

    // ---- モデル読込 ----
    // 例）Assets/Model/LowPolyNature/Mushroom_02.fbx
    // ※ 実行ファイルからの相対パスになるので、実機パスを合わせてください。
    //    うまくいかない時はフルパスで一度確認がおすすめ。
    m_loaded = m_model.Load("Assets/Model/LowPolyNature/Mushroom_01.fbx",
        /*scale*/ 0.02f,
        Model::Flip::None);

    // ---- 使うシェーダを差し替え（ShaderList の既定群）----
    // スキニング不要の通常メッシュは VS_WORLD を使う
    m_model.SetVertexShader(ShaderList::GetVS(ShaderList::VS_WORLD));
    // ランバート拡散＋テクスチャの標準 PS
    m_model.SetPixelShader(ShaderList::GetPS(ShaderList::PS_LAMBERT));

    // 初期カメラ適用＆ライティング
    UpdateCamera(0.0f);
    ApplyWVP();
    ApplyLighting();
}

SceneGame::~SceneGame()
{
}

void SceneGame::Update()
{
    // 入力でグリッド表示切替
    if (IsKeyTrigger(VK_F1)) m_showGrid = !m_showGrid;

    // カメラ自動回転（Enter で停止/再開したい場合はトグル追加も可）
    // とりあえず常時回す：dt はとりづらいので固定刻み
    UpdateCamera(1.0f / 60.0f);
    ApplyWVP();
}

void SceneGame::Draw()
{
    // 3D描画では深度テストON推奨
    SetDepthTest(true);

#ifdef _DEBUG
    if (m_showGrid)
    {
        // デバッググリッド（XZ平面）
        const DirectX::XMFLOAT4 gridCol(0.5f, 0.5f, 0.5f, 1.0f);
        const float size = 20.0f;
        const float step = 1.0f;
        for (float g = -size; g <= size; g += step)
        {
            Geometory::AddLine({ g, 0, -size }, { g, 0,  size }, gridCol);
            Geometory::AddLine({ -size, 0, g }, { size, 0, g }, gridCol);
        }
        // 軸
        Geometory::AddLine({ 0,0,0 }, { 3,0,0 }, { 1,0,0,1 });
        Geometory::AddLine({ 0,0,0 }, { 0,3,0 }, { 0,1,0,1 });
        Geometory::AddLine({ 0,0,0 }, { 0,0,3 }, { 0,0,1,1 });
        Geometory::DrawLines();
    }
#endif

    if (m_loaded)
    {
        // 必要ならここでワールド行列を更新（回転で見せたい場合など）
        DirectX::XMMATRIX W =
            DirectX::XMMatrixRotationY(0.0f) *
            DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
        DirectX::XMStoreFloat4x4(&m_wvp[0], DirectX::XMMatrixTranspose(W));
        ApplyWVP();

        // マテリアル側のテクスチャは Model::Draw が自動バインドしてくれる
        // （マテリアル読み込み時に pTexture がセットされる）
        m_model.Draw();
    }

    // 必要に応じてスプライトやUIをここで描画
}

void SceneGame::UpdateCamera(float dt)
{
    m_camAngle += kCamAngularSpeed * dt;

    const float cx = std::cos(m_camAngle) * kCamRadius;
    const float cz = std::sin(m_camAngle) * kCamRadius;
    const DirectX::XMVECTOR eye = DirectX::XMVectorSet(cx, kCamHeight, cz, 0.0f);
    const DirectX::XMVECTOR at = DirectX::XMVectorSet(0.0f, 0.8f, 0.0f, 0.0f);
    const DirectX::XMVECTOR up = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

    const DirectX::XMMATRIX V =
        DirectX::XMMatrixLookAtLH(eye, at, up);
    const DirectX::XMMATRIX P =
        DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(kFovYDeg),
            16.0f / 9.0f,  // 画面比率。必要なら実ウィンドウサイズに合わせてください
            kZNear, kZFar);

    DirectX::XMStoreFloat4x4(&m_wvp[1], DirectX::XMMatrixTranspose(V));
    DirectX::XMStoreFloat4x4(&m_wvp[2], DirectX::XMMatrixTranspose(P));

    // Geometory 側のカメラにも反映（デバッグライン用）
    Geometory::SetView(m_wvp[1]);
    Geometory::SetProjection(m_wvp[2]);
}

void SceneGame::ApplyWVP()
{
    // ShaderList の VS に world/view/proj を一括で詰める
    ShaderList::SetWVP(m_wvp);
}

void SceneGame::ApplyLighting()
{
    // 適当な平行光源とカメラ位置を設定（Lambert/Specular/Toon で共有）
    ShaderList::SetLight({ 1,1,1,1 }, { -1,-1,-1 });
    ShaderList::SetCameraPos({ 0.0f, kCamHeight, -kCamRadius });
    // フォグが不要なら設定しなくてもOK（PS_FOGのみ使用）
}
