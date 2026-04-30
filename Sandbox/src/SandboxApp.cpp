#include <GameEngine.h>
#include <GameEngine/Core/EntryPoint.h>
#include <imgui/imgui.h>

#include "Scripts/LanternFlickerScript.h"
#include "Scripts/RotatingCrateScript.h"

using namespace GameEngine;

// ---------------------------------------------------------------------------
// Thesis Showcase Scene — FPS Player
//
// Demonstrates all engine systems interactively:
//
//   AssetManager    — LoadMesh auto-loads diffuse textures; no duplicate GPU uploads.
//   ECS             — entities with distinct component combinations.
//   Physics         — dynamic player body controlled by FPSCameraController;
//                     crate falls freely and stops spinning on impact.
//   Lighting        — directional sun + two warm point lights.
//   C++ Scripting   — RotatingCrateScript stops on touch; LanternFlickerScript flickers.
//   Manager order   — Physics → Scene → Game Logic → Render executes correctly.
//
// Controls:
//   Click window    — enter FPS mode (locks cursor)
//   WASD            — move
//   Mouse           — look
//   Space           — jump
//   Shift           — sprint
//   Escape          — exit FPS mode
//
// Required models (place in Sandbox/assets/models/):
//   house.obj   — your existing house model
//   crate.obj   — any wooden crate (kenney.nl, sketchfab free, etc.)
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Scene layout
//
//   z axis points "into screen" (negative = away from player).
//   Player spawns at (0, 1.5, 4) looking toward the house at (0, 0, -6).
//   A crate falls from above and lands in the scene. Walk into it to stop
//   its spin — detected via a velocity spike after the crate has settled.
// ---------------------------------------------------------------------------

static Handle<Scene> BuildScene(AssetManager& assets)
{
    // ---- Load assets -------------------------------------------------------
    // old_house.obj includes the ground plane visually; no separate ground mesh.
    auto houseMesh = assets.LoadMesh("assets/models/old_house.obj");
    auto crateMesh = assets.LoadMesh("assets/models/crate.obj");
    assets.LoadShader("assets/shaders/Mesh.glsl");

    auto scene = MakeHandle<Scene>();

    // -----------------------------------------------------------------------
    // Ground — cooked triangle-mesh collider from dedicated collision mesh.
    // Same rotation as the house (3ds Max Z-up → Y-up).
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("Ground");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, 0.0f, -8.0f };
        t.Rotation    = { glm::radians(-90.0f), 0.0f, 0.0f };

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;

        e.AddComponent<MeshCollider3DComponent>().CollisionMeshPath =
            "assets/models/old_house_ground_collision.obj";
    }

    // -----------------------------------------------------------------------
    // House — visual mesh + cooked triangle-mesh wall collider.
    // Player spawns at Z=+4 looking toward -Z; house sits at Z=-8.
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("House");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, 0.0f, -8.0f };
        t.Rotation    = { glm::radians(-90.0f), 0.0f, 0.0f };

        e.AddComponent<MeshRendererComponent>().Mesh = houseMesh;

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;

        e.AddComponent<MeshCollider3DComponent>().CollisionMeshPath =
            "assets/models/old_house_ground_walls_collision.obj";
    }

    // -----------------------------------------------------------------------
    // Crate — small platform the player can jump on
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("Crate");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { -2.0f, 0.0f, 1.5f }; // sitting on the ground
        t.Rotation    = { 0.0f, 0.0f, 0.0f };
        t.Scale       = { 0.3f, 0.3f, 0.3f };   // 1.5 m cube

        auto& mr = e.AddComponent<MeshRendererComponent>();
        mr.Mesh  = crateMesh;
        mr.Color = { 0.55f, 0.35f, 0.15f, 1.0f };

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Dynamic;
        rb.Mass  = 500.0f;

        // Mesh bbox: X/Z ±2.5, Y 0–5  →  half-extents (2.5,2.5,2.5).
        // Offset centres the physics box on the visual mesh (2.5 × 0.3 = 0.75 m up).
        auto& col       = e.AddComponent<BoxCollider3DComponent>();
        col.HalfExtents = { 2.5f, 2.5f, 2.5f };
        col.Offset      = { 0.0f, 0.75f, 0.0f };

        e.AddComponent<NativeScriptComponent>().Bind<RotatingCrateScript>();
    }

    // -----------------------------------------------------------------------
    // Static crate — solid platform the player can jump on
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("CratePlatform");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 2.0f, 0.0f, 1.5f };
        t.Rotation    = { 0.0f, 0.0f, 0.0f };
        t.Scale       = { 0.3f, 0.3f, 0.3f };

        auto& mr = e.AddComponent<MeshRendererComponent>();
        mr.Mesh  = crateMesh;
        mr.Color = { 0.35f, 0.25f, 0.10f, 1.0f }; // slightly darker to distinguish

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;

        auto& col       = e.AddComponent<BoxCollider3DComponent>();
        col.HalfExtents = { 2.5f, 2.5f, 2.5f };
        col.Offset      = { 0.0f, 0.75f, 0.0f };
    }

    // -----------------------------------------------------------------------
    // Player — thin dynamic box driven by FPSCameraController
    //
    // HalfExtents.y is intentionally small (0.1m) so that Physics3DBody::Position
    // approximates the feet position.  FPSCameraController then adds EyeHeight (1.75m)
    // to get the correct eye point.
    // Low friction keeps the player from sticking to walls or the ramp slope.
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("Player");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, 1.5f, 4.0f }; // start elevated; gravity brings to ground

        auto& rb      = e.AddComponent<Rigidbody3DComponent>();
        rb.Type       = Rigidbody3DComponent::BodyType::Dynamic;
        rb.Mass       = 70.0f;
        rb.Friction   = 0.1f;   // low: horizontal movement is controller-driven, not friction
        rb.Restitution = 0.0f;

        auto& col       = e.AddComponent<BoxCollider3DComponent>();
        col.HalfExtents = { 0.3f, 0.1f, 0.3f }; // narrow box; center ≈ feet position
    }

    // -----------------------------------------------------------------------
    // Lighting
    // -----------------------------------------------------------------------

    // Sun — directional light
    {
        Entity e = scene->CreateEntity("Sun");
        e.GetComponent<TransformComponent>().Rotation = {
            glm::radians(-50.0f), glm::radians(30.0f), 0.0f };

        auto& dl    = e.AddComponent<DirectionalLightComponent>();
        dl.Color     = { 1.00f, 0.95f, 0.85f };
        dl.Intensity = 0.75f;
    }

    // Door lanterns — two warm point lights at house entrance
    {
        auto addLantern = [&](const char* name, glm::vec3 pos)
        {
            Entity e = scene->CreateEntity(name);
            e.GetComponent<TransformComponent>().Translation = pos;

            auto& pl    = e.AddComponent<PointLightComponent>();
            pl.Color     = { 1.0f, 0.65f, 0.25f };
            pl.Intensity = 2.5f;
            pl.Constant  = 1.0f;
            pl.Linear    = 0.14f;
            pl.Quadratic = 0.07f;

            e.AddComponent<NativeScriptComponent>().Bind<LanternFlickerScript>();
        };

        addLantern("Lantern_L", { -1.2f, 2.8f, -7.5f });
        addLantern("Lantern_R", {  1.2f, 2.8f, -7.5f });
    }

    return scene;
}

// ---------------------------------------------------------------------------

class Sandbox : public Application
{
public:
    Sandbox(const ApplicationSpecification& specification)
        : Application(specification),
          myCamera(60.0f, 1280.0f / 720.0f, 0.05f, 500.0f)
    {
        auto& sm = GetSceneManager();
        myScene  = BuildScene(GetAssetManager());

        sm.SetActiveScene(myScene);
        sm.OnRuntimeStart();  // physics world created, bodies registered

        // Connect the FPS camera to the player's physics body so that
        // FPSCameraController drives horizontal movement through PhysX.
        Entity player = myScene->FindEntityByName("Player");
        if (player)
        {
            auto& rb = player.GetComponent<Rigidbody3DComponent>();
            myCamera.SetPhysicsBody(static_cast<Physics3DBody*>(rb.RuntimeBody));
        }

        myCamera.SetPosition({ 0.0f, 1.75f, 4.0f });
        myCamera.SetYaw(-90.0f);
        myCamera.SetFPSMode(true);

        // Cache light entities for ImGui controls.
        mySun      = myScene->FindEntityByName("Sun");
        myLanternL = myScene->FindEntityByName("Lantern_L");
        myLanternR = myScene->FindEntityByName("Lantern_R");

        // Save original intensities so toggles can restore them.
        if (mySun)
            myDirLightIntensity = mySun.GetComponent<DirectionalLightComponent>().Intensity;
        if (myLanternL)
            myPointLightIntensity = myLanternL.GetComponent<PointLightComponent>().Intensity;
    }

    ~Sandbox()
    {
        GetSceneManager().OnRuntimeStop();
    }

protected:
    void OnUpdate(Timestep ts) override
    {
        // Clear framebuffer (color + depth) each frame.
        RenderCommand::SetClearColor({ 0.15f, 0.15f, 0.2f, 1.0f });
        RenderCommand::Clear();

        // 1. FPS camera reads last frame's physics position and writes desired velocity
        //    to Physics3DBody::Velocity.  Physics3D::Step() will pick it up next.
        myCamera.OnUpdate(ts);

        // 2. Scene: physics step (velocity write-back → simulate → sync position),
        //    NativeScriptComponent update, lighting gather.
        if (myScene)
            myScene->OnUpdateRuntime(ts);

        // 3. Render from FPS camera (no primary CameraComponent entity → OnUpdateRuntime
        //    skips its internal render, so this is the only draw call).
        if (myScene)
            myScene->RenderWithCamera(myCamera.GetViewProjection(), myCamera.GetPosition());
    }

    void OnImGuiRender() override
    {
        ImGui::Begin("Thesis Demo");

        // ---- Controls reference ----
        if (ImGui::CollapsingHeader("Controls", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::BulletText("WASD       — move");
            ImGui::BulletText("Mouse      — look");
            ImGui::BulletText("Space      — jump");
            ImGui::BulletText("Shift      — sprint");
            ImGui::BulletText("Escape     — release cursor");
            ImGui::BulletText("Click      — re-enter FPS mode");
        }

        // ---- Lighting ----
        if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Directional light
            if (ImGui::Checkbox("Directional light (Sun)", &myDirLightOn) && mySun)
            {
                mySun.GetComponent<DirectionalLightComponent>().Intensity =
                    myDirLightOn ? myDirLightIntensity : 0.0f;
            }
            if (myDirLightOn && mySun)
            {
                ImGui::SameLine();
                auto& dl = mySun.GetComponent<DirectionalLightComponent>();
                ImGui::SetNextItemWidth(120.0f);
                if (ImGui::SliderFloat("##DirInt", &dl.Intensity, 0.0f, 3.0f))
                    myDirLightIntensity = dl.Intensity;
                ImGui::ColorEdit3("Sun colour", &dl.Color.x,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
            }

            ImGui::Spacing();

            // Point lights
            if (ImGui::Checkbox("Point lights (Lanterns)", &myPointLightsOn))
            {
                float val = myPointLightsOn ? myPointLightIntensity : 0.0f;
                if (myLanternL) myLanternL.GetComponent<PointLightComponent>().Intensity = val;
                if (myLanternR) myLanternR.GetComponent<PointLightComponent>().Intensity = val;
            }
            if (myPointLightsOn && myLanternL)
            {
                ImGui::SameLine();
                auto& pl = myLanternL.GetComponent<PointLightComponent>();
                ImGui::SetNextItemWidth(120.0f);
                if (ImGui::SliderFloat("##PtInt", &pl.Intensity, 0.0f, 6.0f))
                {
                    myPointLightIntensity = pl.Intensity;
                    if (myLanternR)
                        myLanternR.GetComponent<PointLightComponent>().Intensity = pl.Intensity;
                }
                ImGui::ColorEdit3("Lantern colour", &pl.Color.x,
                    ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_Float);
            }
        }

        // ---- Lantern freeze presets ----
        if (ImGui::CollapsingHeader("Lantern Snapshots", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TextDisabled("Freeze flicker for screenshots");
            constexpr float kBright = 4.8f;  // BaseIntensity + FlickerAmount + FlickerAmount*0.5
            constexpr float kDim    = 1.2f;  // BaseIntensity - FlickerAmount - FlickerAmount*0.5

            if (ImGui::Button("Freeze — Brightest"))
                SetLanternFrozen(true, kBright);
            ImGui::SameLine();
            if (ImGui::Button("Freeze — Dimmest"))
                SetLanternFrozen(true, kDim);
            ImGui::SameLine();
            if (ImGui::Button("Resume Flicker"))
                SetLanternFrozen(false, 3.0f);
        }

        // ---- Rendering features ----
        if (ImGui::CollapsingHeader("Rendering", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Checkbox("Normal maps",    &Renderer3D::EnableNormalMaps);
            ImGui::Checkbox("Roughness maps", &Renderer3D::EnableRMAMaps);

            ImGui::Spacing();
            ImGui::TextDisabled("Debug visualisation");
            bool debugNormals = Renderer3D::DebugMode == 1;
            if (ImGui::Checkbox("Show normals as colour", &debugNormals))
                Renderer3D::DebugMode = debugNormals ? 1 : 0;
        }

        ImGui::End();
    }

    void OnUserEvent(Event& e) override
    {
        myCamera.OnEvent(e);

        EventDispatcher dispatcher(e);

        // Click window to re-enter FPS mode if cursor was released.
        dispatcher.Dispatch<MouseButtonPressedEvent>([this](MouseButtonPressedEvent&)
        {
            if (!myCamera.IsFPSMode())
                myCamera.SetFPSMode(true);
            return false;
        });
    }

private:
    Handle<Scene>        myScene;
    FPSCameraController  myCamera;

    // Light entities — cached for ImGui controls.
    Entity myLanternL, myLanternR, mySun;

    // Saved intensities so toggles can restore them.
    float myDirLightIntensity   = 0.75f;
    float myPointLightIntensity = 3.0f;

    // Toggle state.
    bool myDirLightOn    = true;
    bool myPointLightsOn = true;

    void SetLanternFrozen(bool frozen, float intensity)
    {
        for (Entity e : { myLanternL, myLanternR })
        {
            if (!e) continue;
            auto& nsc = e.GetComponent<NativeScriptComponent>();
            if (auto* s = static_cast<LanternFlickerScript*>(nsc.Instance))
            {
                s->Frozen          = frozen;
                s->FrozenIntensity = intensity;
            }
        }
    }
};

// ---------------------------------------------------------------------------

GameEngine::Application* GameEngine::CreateApplication(GameEngine::ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name             = "Thesis Demo — House";
    spec.WorkingDirectory = "../Sandbox";
    spec.CommandLineArgs  = args;
    return new Sandbox(spec);
}
