#include "SpinSystem.h"
#include "../../World.h"
#include "../../Components/TransformComponent.h"

void SpinSystem::Update(World& world, float dt) 
{
    world.View<TransformComponent>([&](EntityId, TransformComponent& tr) 
        {
        tr.rotationDeg.y += ySpeed * dt;
        });
}
