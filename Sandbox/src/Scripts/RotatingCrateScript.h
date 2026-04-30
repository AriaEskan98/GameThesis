#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

// Crate sits still until the player hits it, then spins permanently on the Y axis.
class RotatingCrateScript : public GameEngine::ScriptableEntity
{
public:
    float RotationSpeed = 1.5f;

protected:
    void OnUpdate(GameEngine::Timestep ts) override
    {
        if (mySpinning)
        {
            auto& t = GetComponent<GameEngine::TransformComponent>();
            t.Rotation.y += RotationSpeed * (float)ts;
            return;
        }

        auto& rb   = GetComponent<GameEngine::Rigidbody3DComponent>();
        auto* body = static_cast<GameEngine::Physics3DBody*>(rb.RuntimeBody);
        if (!body)
            return;

        float speed = glm::length(body->Velocity);

        // Wait for the crate to settle after spawning, then watch for a hit.
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
            mySpinning = true;
        }
    }

private:
    float mySettledTime = 0.0f;
    bool  myHasSettled  = false;
    bool  mySpinning    = false;
};
