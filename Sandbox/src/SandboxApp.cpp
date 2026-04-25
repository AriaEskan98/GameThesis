#include <GameEngine.h>
#include <GameEngine/Core/EntryPoint.h>
#include <imgui/imgui.h>

#include "Scripts/LanternFlickerScript.h"
#include "Scripts/RotatingCrateScript.h"

using namespace GameEngine;

// ---------------------------------------------------------------------------
// Thesis Showcase Scene — FPS Player + Staircase
//
// Demonstrates all engine systems interactively:
//
//   AssetManager    — LoadMesh auto-loads diffuse textures; no duplicate GPU uploads.
//   ECS             — entities with distinct component combinations.
//   Physics         — dynamic player body controlled by FPSCameraController;
//                     crate falls freely; ramp has a rotated static collider.
//   Lighting        — directional sun + two warm point lights.
//   C++ Scripting   — (removed orbit camera; player is now FPSCameraController)
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
//   ground.obj  — any flat ground / grass plane
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Scene layout
//
//   z axis points "into screen" (negative = away from player).
//   Player spawns at (0, 1.5, 4) looking toward the house at (0, 0, -6).
//   A 5-step staircase sits between the player and the house entrance.
//   The staircase has:
//     • visual step meshes (MeshRendererComponent, NO collider)
//     • one invisible ramp box (Rigidbody Static, rotated collider, NO mesh)
//   This lets the player walk smoothly up the ramp while the steps look correct.
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
    // Ground — invisible static physics plane (visual ground is part of house mesh)
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("Ground");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, -0.1f, 0.0f };
        t.Scale       = { 20.0f, 0.2f, 20.0f };

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;

        auto& col = e.AddComponent<BoxCollider3DComponent>();
        // Use default HalfExtents (0.5,0.5,0.5); physics box = 0.5 * Scale = (10, 0.1, 10) — 20cm thick slab.
    }

    // -----------------------------------------------------------------------
    // House — static mesh + wall collider, placed 10 units ahead of spawn.
    // Player spawns at Z=+4 looking toward -Z; house sits at Z=-8.
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("House");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, 0.0f, -8.0f };
        t.Rotation    = { glm::radians(90.0f), 0.0f, 0.0f }; // Fix 3ds Max Z-up → Y-up

        e.AddComponent<MeshRendererComponent>().Mesh = houseMesh;

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;

        // Rough bounding box for the house walls (player can't walk through).
        auto& col       = e.AddComponent<BoxCollider3DComponent>();
        col.HalfExtents = { 5.0f, 3.0f, 5.0f };
    }

    // -----------------------------------------------------------------------
    // Staircase
    //
    // 5 steps, each 0.25 m tall × 0.6 m deep × 3 m wide.
    // Total rise: 1.25 m   Total run: 3.0 m
    //
    // Visual steps (mesh only, no physics) — sit on top of the ramp.
    // Ramp collider (rotated static box, no mesh) — player walks up it.
    // -----------------------------------------------------------------------
    {
        // -- Visual steps (decorative, no collider) --
        // 5 steps leading from Z=-2 toward the house at Z=-8.
        // Each step: 0.25 m tall, 0.6 m deep, 3 m wide.
        for (int i = 0; i < 5; ++i)
        {
            Entity e = scene->CreateEntity("Step_" + std::to_string(i));
            auto& t  = e.GetComponent<TransformComponent>();
            t.Translation = { 0.0f,
                              0.125f + i * 0.25f,
                             -2.0f  - i * 0.6f  };
            t.Scale = { 3.0f, 0.25f, 0.6f };

            auto& mr = e.AddComponent<MeshRendererComponent>();
            mr.Mesh  = crateMesh;
            mr.Color = { 0.55f, 0.35f, 0.15f, 1.0f }; // wood brown
        }

        // -- Ramp collider (invisible — no MeshRendererComponent) --
        // rise = 1.25 m over run = 3.0 m  →  angle ≈ 22.6°
        // Center of ramp between bottom (z=-2) and top (z=-5):  z=-3.5
        {
            constexpr float kAngle = glm::radians(22.6f);

            Entity ramp = scene->CreateEntity("StairRamp");
            auto& t     = ramp.GetComponent<TransformComponent>();
            t.Translation = { 0.0f, 0.625f, -3.5f };
            t.Rotation    = { -kAngle, 0.0f, 0.0f };
            t.Scale       = { 3.0f, 0.3f, 3.25f };

            auto& rb = ramp.AddComponent<Rigidbody3DComponent>();
            rb.Type  = Rigidbody3DComponent::BodyType::Static;
        }
    }

    // -----------------------------------------------------------------------
    // Falling crate — dynamic rigid body, spawned above the scene
    // -----------------------------------------------------------------------
    {
        Entity e = scene->CreateEntity("Crate");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { -2.5f, 5.0f, -1.0f };
        t.Rotation    = { 0.2f, 0.5f, 0.1f };

        auto& mr = e.AddComponent<MeshRendererComponent>();
        mr.Mesh  = crateMesh;
        mr.Color = { 0.55f, 0.35f, 0.15f, 1.0f }; // wood brown
        e.AddComponent<Rigidbody3DComponent>();   // Dynamic, UseGravity = true by default
        e.AddComponent<BoxCollider3DComponent>();
        e.AddComponent<NativeScriptComponent>().Bind<RotatingCrateScript>();
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
        myCamera.SetYaw(-90.0f);   // look toward house (-Z direction)
        myCamera.SetFPSMode(true); // lock cursor and start in FPS mode
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
        ImGui::Text("Scene: House Exterior + Staircase");
        ImGui::Separator();
        ImGui::Text("Controls:");
        ImGui::BulletText("WASD — move");
        ImGui::BulletText("Mouse — look around");
        ImGui::BulletText("Space — jump");
        ImGui::BulletText("Shift — sprint");
        ImGui::BulletText("Escape — release cursor");
        ImGui::Separator();
        ImGui::Text("Active systems:");
        ImGui::BulletText("AssetManager  — textures auto-loaded from meshes");
        ImGui::BulletText("Physics3D     — player, crate, ramp all simulated");
        ImGui::BulletText("FPSCamera     — drives player via Physics3DBody");
        ImGui::BulletText("NativeScript  — crate rotates, lanterns flicker");
        ImGui::BulletText("Renderer3D    — Blinn-Phong, 1 dir + 2 point lights");
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
};

// ---------------------------------------------------------------------------

GameEngine::Application* GameEngine::CreateApplication(GameEngine::ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name             = "Thesis Demo — House + Staircase";
    spec.WorkingDirectory = "../Sandbox";
    spec.CommandLineArgs  = args;
    return new Sandbox(spec);
}
