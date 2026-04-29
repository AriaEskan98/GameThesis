#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

// Spins the crate on the Y axis. Stops permanently the first time the crate is
// hit after it has settled on the ground.
//
// Detection: once the crate has rested at near-zero speed for 0.5 s (myHasSettled),
// any subsequent velocity above 0.2 m/s means a collision. The settled flag is
// never cleared, so brief jitter after landing can't prevent detection.
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

            if (!myHasSettled)
            {
                if (speed < 0.05f)
                    mySettledTime += (float)ts;
                else
                    mySettledTime = 0.0f;

                if (mySettledTime > 0.5f)
                    myHasSettled = true;
            }
            else if (speed > 0.2f)
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
    bool  myHasSettled  = false;
    bool  mySpinning    = true;
};
