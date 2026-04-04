#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

/// Smoothly orbits the camera around a world-space point.
///
/// Attach to an entity that also has a CameraComponent (Primary = true).
/// The script updates the entity's TransformComponent each frame so that
/// Scene::OnUpdateRuntime picks it up as the active view.
///
/// Demonstrates: NativeScriptComponent, ScriptableEntity lifecycle,
///               GetComponent<T>(), automatic TransformComponent manipulation.
class OrbitCameraScript : public GameEngine::ScriptableEntity
{
public:
    // ---------- configuration (set before runtime start) ----------
    glm::vec3 Target   = { 0.0f, 1.5f, -4.0f }; ///< World point the camera looks at.
    float     Radius   = 10.0f;                   ///< Orbit radius in world units.
    float     Height   = 4.5f;                    ///< Camera height in world units.
    float     Speed    = 0.35f;                   ///< Orbit speed in radians/second.

protected:
    void OnCreate() override
    {
        // Start the camera behind and to the side of the scene
        myAngle = glm::radians(30.0f);
        UpdateTransform(myAngle);
    }

    void OnUpdate(GameEngine::Timestep ts) override
    {
        myAngle += ts * Speed;
        UpdateTransform(myAngle);
    }

private:
    void UpdateTransform(float angle)
    {
        auto& t = GetComponent<GameEngine::TransformComponent>();

        // --- Position ---
        t.Translation = {
            Target.x + glm::cos(angle) * Radius,
            Height,
            Target.z + glm::sin(angle) * Radius
        };

        // --- Rotation: look toward Target ---
        // After a Y-axis rotation by `yaw`, the camera's forward direction (-Z) becomes
        // (-sin(yaw), 0, -cos(yaw)).  We want that to equal the horizontal unit vector
        // from camera to Target, giving: yaw = pi/2 - angle.
        float yaw   =  glm::half_pi<float>() - angle;
        float pitch = -glm::atan(Height - Target.y, Radius); // look slightly down
        t.Rotation  = { pitch, yaw, 0.0f };
    }

    float myAngle = 0.0f;
};
