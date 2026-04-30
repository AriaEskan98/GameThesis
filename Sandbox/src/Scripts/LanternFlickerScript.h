#pragma once

#include <GameEngine.h>
#include <glm/glm.hpp>

/// Animates a PointLightComponent's intensity each frame using layered sine waves,
/// producing a convincing fire-flicker effect.
///
/// Demonstrates: NativeScriptComponent, OnUpdate lifecycle, GetComponent<T>()
/// writing back to an ECS component every frame.
class LanternFlickerScript : public GameEngine::ScriptableEntity
{
public:
    float BaseIntensity = 3.0f;  ///< Resting brightness.
    float FlickerAmount = 1.2f;  ///< Peak deviation from base.

protected:
    void OnUpdate(GameEngine::Timestep ts) override
    {
        myTime += (float)ts;

        // Two out-of-phase sines at non-harmonic frequencies give an
        // irregular, flame-like flicker without a visible pattern.
        float flicker = FlickerAmount * glm::sin(myTime * 7.3f)
                      + (FlickerAmount * 0.5f) * glm::sin(myTime * 13.1f);

        GetComponent<GameEngine::PointLightComponent>().Intensity =
            BaseIntensity + flicker;
    }

private:
    float myTime = 0.0f;
};
