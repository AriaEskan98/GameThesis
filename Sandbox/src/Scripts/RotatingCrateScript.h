#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

/// Slowly spins the crate on the Y axis once it has settled on the ground.
///
/// Demonstrates: NativeScriptComponent, OnUpdate lifecycle, reading and writing
/// TransformComponent from a script — the simplest possible scripted behaviour.
class RotatingCrateScript : public GameEngine::ScriptableEntity
{
public:
    float RotationSpeed = 1.0f; ///< Radians per second.

protected:
    void OnUpdate(GameEngine::Timestep ts) override
    {
        auto& t = GetComponent<GameEngine::TransformComponent>();
        t.Rotation.y += RotationSpeed * (float)ts;
    }
};
