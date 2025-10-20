#include "PrefabRegistry.h"

void PrefabRegistry::Register(const std::string& name, SpawnFunc fn)
{
    m_map[name] = std::move(fn);
}

EntityId PrefabRegistry::Spawn(const std::string& name, World& world, const SpawnParams& params) const
{
    auto it = m_map.find(name);
    if (it == m_map.end()) return kInvalidEntity;
    return it->second(world, params);
}

bool PrefabRegistry::Has(const std::string& name) const
{
    return m_map.find(name) != m_map.end();
}
