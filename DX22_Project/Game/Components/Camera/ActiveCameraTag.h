#pragma once
/**
 * @file ActiveCameraTag.h
 * @brief 現在アクティブなカメラであることを示すタグ（1つだけ付与）
 * @details
 * - Render 側に View/Proj を渡すシステムが、このタグのついたカメラを探す前提。
 * - シーン切替やカメラ切替時は付け替えるだけでOK。
 */
struct ActiveCameraTag {};
