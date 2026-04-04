# GameEngine — Architecture & Design Decisions

This document describes the goals of the engine, the architectural decisions made to achieve them, and how each system works. It is intended as the technical reference for the thesis.

---

## Project Goals

The engine is a **3D-only game engine** written in C++17, targeting Windows with OpenGL as the current rendering backend. It is designed with the following specific goals:

1. **Interchangeable rendering API** — the rendering abstraction layer makes it possible to swap OpenGL for Vulkan without changing game code.
2. **Structured update order** — systems execute in a predictable, Unity-style order each frame.
3. **Automatic asset management** — loading a mesh automatically loads its textures; the same file is never loaded from disk or uploaded to the GPU more than once.
4. **C++ scripting** — game logic attaches to entities via C++ script classes, the same concept as Unity's MonoBehaviour but without a managed runtime.
5. **Industry-standard physics** — NVIDIA PhysX replaces a custom solver, providing accurate rigid body simulation, collision detection, and a foundation for character controllers and climbing.

---

## 1. Interchangeable Rendering API

### Goal
The renderer should be backend-agnostic. Switching from OpenGL to Vulkan should require no changes to game code or Scene code.

### How it works
All GPU operations go through an abstraction layer:

```
Game code
  ↓
Renderer3D / RenderCommand        ← high-level calls (DrawIndexed, SetViewport, etc.)
  ↓
RendererAPI (abstract base class)  ← defines the interface
  ↓
OpenGLRendererAPI                  ← current concrete implementation
```

Key files:
- `GameEngine/Renderer/RendererAPI.h` — abstract interface (`DrawIndexed`, `SetClearColor`, etc.)
- `Platform/OpenGL/OpenGLRendererAPI.cpp` — OpenGL implementation
- `GameEngine/Renderer/RenderCommand.h` — static wrapper used by all render code

To add Vulkan: implement `VulkanRendererAPI` and add a case to the `RendererAPI::Create()` factory. No other code changes.

---

## 2. Unity-Style Update Order (Manager Architecture)

### Goal
Engine systems should execute in a fixed, predictable order every frame, matching industry conventions (Physics → Scene → Game Logic → Render → UI).

### How it works
`Application` owns five manager objects and calls them in order inside `Run()`:

```cpp
myPhysicsManager->Update(ts);   // 1. Physics   — rigid bodies, collision
mySceneManager->Update(ts);     // 2. Scene     — animation, scene systems
OnUpdate(ts);                   // 3. Game logic — user's code (override in subclass)
myRenderManager->Render();      // 4. Render    — 3D draw calls
myUIManager->Render(...);       // 5. UI        — ImGui, always last
```

Equivalent to Unity's:
```
FixedUpdate  → physics
Update       → game logic
LateUpdate   → post-logic
Render       → draw
```

| Manager | Responsibility | Status |
|---|---|---|
| `PhysicsManager` | PhysX world step | Stub — PhysX ticks inside Scene for now |
| `SceneManager` | Scene lifecycle | Active — OnRuntimeStart/Stop, holds active Scene |
| `RenderManager` | 3D render pass | Stub — rendering is in Scene/Renderer3D |
| `UIManager` | ImGui context | Active — owns ImGuiLayer, wraps Begin/End |
| `AssetManager` | Asset caching | Active — meshes, textures, shaders |

Key files:
- `GameEngine/Managers/` — all manager classes
- `GameEngine/Core/Application.h/.cpp` — owns and orchestrates managers

---

## 3. Asset Manager — No Duplicate Loading

### Goal
Loading the same mesh, texture, or shader file twice should return the same object. No duplicate disk reads, no duplicate GPU uploads.

### How it works
`AssetManager` is owned by `Application` (initialized first, shut down last). It holds three caches keyed by file path or name:

```
AssetManager
  ├── TextureLibrary  → unordered_map<path, Handle<Texture2D>>
  ├── ShaderLibrary   → unordered_map<name, Handle<Shader>>
  └── Mesh cache      → unordered_map<path, Handle<Mesh>>
```

Any call to `Load*` checks the cache first:

```cpp
// Game code — load once, share everywhere:
auto& assets = Application::GetInstance().GetAssetManager();
auto chair   = assets.LoadMesh("assets/models/chair.obj");
auto shader  = assets.LoadShader("assets/shaders/Mesh.glsl");
```

### Automatic texture loading
When a mesh is loaded, its diffuse texture is extracted from the Assimp material table and loaded automatically through `AssetManager`:

```
assets.LoadMesh("chair.obj")
  → Mesh::Create()         — Assimp loads geometry + UV coords
  → reads material table   — finds "wood.png"
  → Renderer3D::LoadTexture("wood.png")
      → AssetManager::LoadTexture()  ← cached; returns same Handle on subsequent calls

Renderer3D::Submit(mesh)
  → binds mesh->GetTexture()  or 1×1 white fallback if no texture
  → draws
```

The game developer loads a mesh — textures come along automatically. No manual texture calls.

Key files:
- `GameEngine/Managers/AssetManager.h/.cpp`
- `GameEngine/Renderer/Mesh.cpp` — texture extraction from Assimp materials
- `GameEngine/Renderer/Renderer3D.cpp` — default white texture, texture binding in Submit()
- `GameEngine/Renderer/Texture.h` — `TextureLibrary` class
- `GameEngine/Renderer/Shader.h` — `ShaderLibrary` class

---

## 4. Entity-Component System (ECS)

### Goal
Game objects are data-driven. Components store state; systems process components. No deep inheritance trees.

### How it works
The engine uses [EnTT](https://github.com/skypjack/entt), a high-performance C++ ECS library.

```cpp
// Create an entity
Entity door = scene.CreateEntity("Door");

// Add components
door.AddComponent<TransformComponent>(glm::vec3(5, 0, 0));
door.AddComponent<MeshRendererComponent>();
door.AddComponent<Rigidbody3DComponent>();
door.AddComponent<NativeScriptComponent>().Bind<DoorController>();
```

Available 3D components:

| Component | Purpose |
|---|---|
| `TransformComponent` | Position, rotation, scale |
| `CameraComponent` | Entity-attached camera |
| `MeshRendererComponent` | Attaches a `Handle<Mesh>` to an entity |
| `NativeScriptComponent` | Attaches a C++ script class |
| `Rigidbody3DComponent` | PhysX dynamic/static/kinematic body |
| `BoxCollider3DComponent` | AABB collider |
| `DirectionalLightComponent` | Sun light |
| `PointLightComponent` | Positional light with attenuation |

Key files:
- `GameEngine/Scene/Components.h` — all component structs
- `GameEngine/Scene/Scene.cpp` — ECS iteration (update, physics, render)
- `GameEngine/Scene/Entity.h` — typed wrapper around entt::entity

---

## 5. C++ Scripting

### Goal
Game logic attaches to entities via C++ classes (like Unity's MonoBehaviour). No managed runtime (C# / Mono was removed).

### How it works
A script inherits from `ScriptableEntity` and overrides lifecycle methods:

```cpp
class DoorController : public GameEngine::ScriptableEntity
{
public:
    void OnCreate() override
    {
        // Called once when the scene starts
    }

    void OnUpdate(Timestep ts) override
    {
        auto& transform = GetComponent<TransformComponent>();
        // Check player distance, open door, play animation, etc.
    }
};
```

Bind to an entity:
```cpp
doorEntity.AddComponent<NativeScriptComponent>().Bind<DoorController>();
```

The `Scene` calls `OnCreate` once at runtime start and `OnUpdate` every frame for all entities with `NativeScriptComponent`. The script can call `GetComponent<T>()` to read and write any component on its entity.

Key files:
- `GameEngine/Scene/ScriptableEntity.h` — base class
- `GameEngine/Scene/Components.h` — `NativeScriptComponent`
- `GameEngine/Scene/Scene.cpp` — lifecycle calls in `OnRuntimeStart` / `OnUpdateRuntime`

---

## 6. Physics — NVIDIA PhysX

### Goal
Replace the custom AABB solver with an industry-standard physics engine that supports accurate rigid bodies, joints, character controllers, and is a foundation for features like climbing.

### How it works
`Physics3DWorld` wraps a PhysX `PxScene`. The interface is identical to the old custom solver so `Scene.cpp` required no changes.

```
Physics3DWorld::CreateBody(def)
  → PxRigidStatic   if Mass == 0
  → PxRigidDynamic  if Mass > 0  (kinematic flag respected)

Physics3DWorld::Step(dt)
  → PxScene::simulate(dt)
  → PxScene::fetchResults()
  → sync Position / Velocity back to Physics3DBody
  → Scene reads Physics3DBody::Position → updates TransformComponent
```

PhysX is vendored as pre-built static libs (`/MD` runtime to match the rest of the project) under `GameEngine/vendor/PhysX/`.

Key files:
- `GameEngine/Physics/Physics3D.h/.cpp` — PhysX wrapper
- `GameEngine/Scene/Components.h` — `Rigidbody3DComponent`, `BoxCollider3DComponent`
- `GameEngine/Scene/Scene.cpp` — `OnPhysics3DStart/Stop`, physics update loop

---

## 7. Lighting Model — Blinn-Phong

### How it works
Lighting is computed in the fragment shader using Blinn-Phong shading. Data is passed via Uniform Buffer Objects (UBOs) at fixed binding points:

| Binding | UBO | Contents |
|---|---|---|
| 1 | CameraData | ViewProjection matrix, camera world position |
| 2 | ObjectData | Model transform, flat colour, entity ID |
| 3 | LightData | Ambient colour, directional light, up to 4 point lights |

Supported lights:
- **DirectionalLightComponent** — sun light, parallel rays, single instance per scene
- **PointLightComponent** — positional light with quadratic attenuation, up to 4 per scene

Diffuse texture is sampled at binding 0, multiplied into the final colour. A default 1×1 white texture is used for untextured meshes.

Key files:
- `Sandbox/assets/shaders/Mesh.glsl` — full Blinn-Phong vertex + fragment shader
- `GameEngine/Renderer/Renderer3D.h/.cpp` — UBO upload and draw submission

---

## 8. Event System

### How it works
Events are dispatched immediately (blocking). The dispatcher matches events to typed handlers using C++ templates.

```cpp
void Sandbox::OnUserEvent(Event& e)
{
    EventDispatcher dispatcher(e);
    dispatcher.Dispatch<KeyPressedEvent>([this](KeyPressedEvent& e)
    {
        // handle key press
        return true; // mark as handled — stops propagation
    });
}
```

Event categories use bitflags so one event can belong to multiple categories:
```cpp
e.IsInCategory(EventCategoryInput);    // true for mouse + keyboard events
e.IsInCategory(EventCategoryKeyboard); // true only for keyboard events
```

UI gets event priority: `UIManager::OnEvent(e)` is called first; if ImGui consumes it, the game never sees it.

Key files:
- `GameEngine/Events/Event.h` — base class, dispatcher, `DECLARE_EVENT_TYPE` macro
- `GameEngine/Events/ApplicationEvent.h`, `KeyEvent.h`, `MouseEvent.h`

---

## 9. What Was Removed and Why

| Removed | Reason |
|---|---|
| 2D Renderer (Renderer2D) | Engine is 3D-only |
| Box2D / 2D physics components | Replaced by PhysX 3D |
| C# scripting (Mono / ScriptEngine) | Heavy dependency, complex setup; C++ scripts are sufficient |
| Layer / LayerStack system | Replaced by direct virtual hooks in Application subclass |
| AudioManager | Out of scope for this thesis |
| Font / MSDF text rendering | No text needed for 3D gameplay |
| OrthographicCameraController | 2D input controller; OrthographicCamera kept (useful for shadow mapping) |

---

## Summary — What the Game Developer Actually Writes

```cpp
class MyGame : public GameEngine::Application
{
    Handle<Scene>  myScene;
    Handle<Mesh>   myMesh;

    void OnUpdate(Timestep ts) override
    {
        myScene->OnUpdateRuntime(ts);
    }
};

// In scene setup:
auto& assets = Application::GetInstance().GetAssetManager();
auto mesh    = assets.LoadMesh("assets/models/chair.obj"); // texture auto-loaded
auto shader  = assets.LoadShader("assets/shaders/Mesh.glsl");

auto chair = scene.CreateEntity("Chair");
chair.AddComponent<TransformComponent>(glm::vec3(0, 0, 0));
chair.AddComponent<MeshRendererComponent>().Mesh = mesh;
chair.AddComponent<Rigidbody3DComponent>();
chair.AddComponent<NativeScriptComponent>().Bind<ChairScript>();
```

The developer works with entities and components. Asset loading, texture binding, physics stepping, and render submission are all handled by the engine.
