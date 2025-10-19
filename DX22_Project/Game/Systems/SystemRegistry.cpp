#include "SystemRegistry.h"
#include "../World.h"

void SystemRegistry::Tick(World& world, float dt)
{
    for (auto& s : m_updates) s->Update(world, dt);
}

void SystemRegistry::Render(World& world)
{
    for (auto& s : m_renders) s->Render(world);
}
