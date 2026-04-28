#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

// Spins the crate on the Y axis. Stops permanently the first time the crate is
// hit after it has settled on the ground.
//
// Detection: Physics3DBody::Velocity is synced from PhysX every frame. Once the
// crate has rested at near-zero speed for 1.5 s, any velocity spike above 0.4 m/s
// means something collided with it.
class RotatingCrateScript : public GameEngine::ScriptableEntity
{
public:
    float RotationSpeed = 1.0f;

protected:
    void OnUpdate(GameEngine::Timestep ts) override
    {
        if (!mySpinning)
            return;

        auto& rb   = GetComponent<GameEngine::Rigidbody3DComponent>();
        auto* body = static_cast<GameEngine::Physics3DBody*>(rb.RuntimeBody);

        if (body)
        {
            float speed = glm::length(body->Velocity);

            if (speed < 0.05f)
                mySettledTime += (float)ts;
            else
                mySettledTime = 0.0f;

            if (mySettledTime > 1.5f && speed > 0.4f)
            {
                mySpinning = false;
                return;
            }
        }

        auto& t = GetComponent<GameEngine::TransformComponent>();
        t.Rotation.y += RotationSpeed * (float)ts;
    }

private:
    float mySettledTime = 0.0f;
    bool  mySpinning    = true;
};
