#include <GameEngine.h>
#include <GameEngine/Core/EntryPoint.h>

#include "Scripts/OrbitCameraScript.h"

using namespace GameEngine;

// ---------------------------------------------------------------------------
// Thesis showcase scene
//
// Demonstrates all engine systems in a single exterior scene:
//   - AssetManager:   LoadMesh auto-loads diffuse textures; no duplicate GPU uploads.
//   - ECS:            Seven entities, each with a distinct component mix.
//   - Physics:        Crate falls from height and lands on the static ground.
//   - Lighting:       Directional sun + two warm point lights (door lanterns).
//   - C++ Scripting:  OrbitCameraScript moves the camera entity each frame.
//   - Manager order:  Physics → Scripts → Render executes in the correct sequence.
//
// Required models (place in Sandbox/assets/models/):
//   house.obj   — your existing house model
//   crate.obj   — any wooden crate (kenney.nl, sketchfab free, etc.)
//   ground.obj  — any flat ground / grass plane
// ---------------------------------------------------------------------------

static Handle<Scene> BuildScene(AssetManager& assets)
{
    // ---- Load assets -------------------------------------------------------
    // Calling LoadMesh once per path is enough — the AssetManager caches them.
    // Each mesh also automatically loads its diffuse texture through the cache.
    auto houseMesh  = assets.LoadMesh("assets/models/house.obj");
    auto crateMesh  = assets.LoadMesh("assets/models/crate.obj");
    auto groundMesh = assets.LoadMesh("assets/models/ground.obj");
    assets.LoadShader("assets/shaders/Mesh.glsl"); // pre-warm shader cache

    // ---- Scene -------------------------------------------------------------
    auto scene = MakeHandle<Scene>();

    // -- Ground (static rigid body) ------------------------------------------
    // A large flat surface so the falling crate has somewhere to land.
    {
        Entity e = scene->CreateEntity("Ground");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Scale  = { 5.0f, 0.2f, 5.0f };

        e.AddComponent<MeshRendererComponent>().Mesh = groundMesh;

        auto& rb  = e.AddComponent<Rigidbody3DComponent>();
        rb.Type   = Rigidbody3DComponent::BodyType::Static;

        auto& col        = e.AddComponent<BoxCollider3DComponent>();
        col.HalfExtents  = { 5.0f, 0.1f, 5.0f };
    }

    // -- House (static mesh + static collider) --------------------------------
    // Placed behind the scene centre so the orbit camera frames it nicely.
    {
        Entity e = scene->CreateEntity("House");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 0.0f, 0.0f, -4.0f };

        e.AddComponent<MeshRendererComponent>().Mesh = houseMesh;

        auto& rb = e.AddComponent<Rigidbody3DComponent>();
        rb.Type  = Rigidbody3DComponent::BodyType::Static;
        e.AddComponent<BoxCollider3DComponent>();
    }

    // -- Falling crate (dynamic rigid body) -----------------------------------
    // Spawns above the scene and falls under gravity — shows PhysX in action.
    {
        Entity e = scene->CreateEntity("Crate");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Translation = { 1.5f, 6.0f, -2.5f };
        t.Rotation    = { 0.2f, 0.5f, 0.1f }; // slight tilt so it tumbles

        e.AddComponent<MeshRendererComponent>().Mesh = crateMesh;
        e.AddComponent<Rigidbody3DComponent>();  // Dynamic, UseGravity = true by default
        e.AddComponent<BoxCollider3DComponent>();
    }

    // -- Sun (directional light) ----------------------------------------------
    // Rotation determines the light direction (euler angles in radians).
    {
        Entity e = scene->CreateEntity("Sun");
        auto& t  = e.GetComponent<TransformComponent>();
        t.Rotation = { glm::radians(-50.0f), glm::radians(30.0f), 0.0f };

        auto& dl    = e.AddComponent<DirectionalLightComponent>();
        dl.Color     = { 1.00f, 0.95f, 0.85f }; // warm daylight
        dl.Intensity = 0.75f;
    }

    // -- Door lanterns (point lights) -----------------------------------------
    // Two warm orange-yellow lights placed either side of the house entrance.
    // Shows the engine supports up to 4 simultaneous point lights.
    {
        auto makeLantern = [&](const char* name, glm::vec3 pos)
        {
            Entity e = scene->CreateEntity(name);
            e.GetComponent<TransformComponent>().Translation = pos;

            auto& pl    = e.AddComponent<PointLightComponent>();
            pl.Color     = { 1.0f, 0.65f, 0.25f }; // fire-orange
            pl.Intensity = 2.5f;
            pl.Constant  = 1.0f;
            pl.Linear    = 0.14f;
            pl.Quadratic = 0.07f;
        };

        makeLantern("Lantern_L", { -1.2f, 2.8f, -2.0f });
        makeLantern("Lantern_R", {  1.2f, 2.8f, -2.0f });
    }

    // -- Camera entity (orbit script) -----------------------------------------
    // The NativeScriptComponent runs OrbitCameraScript every frame.
    // Scene::OnUpdateRuntime finds this as the primary camera and renders from it.
    {
        Entity e = scene->CreateEntity("Camera");

        auto& cam = e.AddComponent<CameraComponent>();
        cam.Primary = true;
        cam.Camera.SetPerspective(glm::radians(60.0f), 0.1f, 500.0f);

        auto& nsc = e.AddComponent<NativeScriptComponent>();
        nsc.Bind<OrbitCameraScript>();
    }

    return scene;
}

// ---------------------------------------------------------------------------

class Sandbox : public Application
{
public:
    Sandbox(const ApplicationSpecification& specification)
        : Application(specification)
    {
        auto& sm = GetSceneManager();
        myScene  = BuildScene(GetAssetManager());

        sm.SetActiveScene(myScene);
        sm.OnRuntimeStart();  // calls Scene::OnRuntimeStart → physics world init,
                              // NativeScriptComponent::OnCreate for OrbitCameraScript
    }

    ~Sandbox()
    {
        GetSceneManager().OnRuntimeStop();
    }

protected:
    void OnUpdate(Timestep ts) override
    {
        // All systems run here:
        //   PhysicsManager::Update  → already ticked by Application before OnUpdate
        //   Scene::OnUpdateRuntime  → scripts + physics sync + render
        if (myScene)
            myScene->OnUpdateRuntime(ts);
    }

    void OnImGuiRender() override
    {
        ImGui::Begin("Thesis Demo");
        ImGui::Text("Scene: House Exterior");
        ImGui::Separator();
        ImGui::Text("ECS entities: Ground, House, Crate, Sun,");
        ImGui::Text("              Lantern_L, Lantern_R, Camera");
        ImGui::Separator();
        ImGui::Text("Systems active:");
        ImGui::BulletText("AssetManager  — textures auto-loaded from meshes");
        ImGui::BulletText("Physics3D     — crate falls with gravity");
        ImGui::BulletText("NativeScript  — OrbitCameraScript drives camera");
        ImGui::BulletText("Renderer3D    — Blinn-Phong, 1 dir + 2 point lights");
        ImGui::End();
    }

    void OnUserEvent(Event& e) override
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& ke)
        {
            if (ke.GetKeyCode() == Key::Escape)
                Close();
            return false;
        });
    }

private:
    Handle<Scene> myScene;
};

// ---------------------------------------------------------------------------

GameEngine::Application* GameEngine::CreateApplication(GameEngine::ApplicationCommandLineArgs args)
{
    ApplicationSpecification spec;
    spec.Name             = "Thesis Demo — House Exterior";
    spec.WorkingDirectory = "../Sandbox";
    spec.CommandLineArgs  = args;
    return new Sandbox(spec);
}
