#pragma once
#include <string>

/**
 * @file NameComponent.h
 * @brief デバッグ・検索用の表示名（ユニークである必要はない）
 */
struct NameComponent 
{
    std::string value;
    explicit NameComponent(const std::string& n = {}) : value(n) {}
};
