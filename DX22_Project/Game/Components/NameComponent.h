#pragma once
#include <string>

/**
 * @file NameComponent.h
 * @brief �f�o�b�O�E�����p�̕\�����i���j�[�N�ł���K�v�͂Ȃ��j
 */
struct NameComponent 
{
    std::string value;
    explicit NameComponent(const std::string& n = {}) : value(n) {}
};
