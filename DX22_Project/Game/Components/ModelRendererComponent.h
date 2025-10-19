#pragma once
class Model; // 重いヘッダの依存回避：前方宣言だけにする

/**
 * @brief 3Dモデル描画コンポーネント
 * @details
 * - model が nullptr の場合はスキップ
 * - 表示フラグ visible で簡易ON/OFF
 * - 将来的にマテリアル上書き/色乗算などを追加しやすい
 */
struct ModelRendererComponent
{
    Model* model = nullptr;
    bool   visible = true;
};

// 既存コード互換
using ModelRenderer = ModelRendererComponent;
