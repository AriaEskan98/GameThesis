#pragma once

#include "Hazel/Core/Timestep.h"
#include "Hazel/Core/UUID.h"
#include "Hazel/Renderer/EditorCamera.h"

#include "entt.hpp"

class b2World;

namespace GameEngine {

class Physics3DWorld;

	class Entity;

	class Scene
	{
	public:
		Scene();
		~Scene();

		static Handle<Scene> Copy(Handle<Scene> other);

		Entity CreateEntity(const std::string& name = std::string());
		Entity CreateEntityWithUUID(UUID uuid, const std::string& name = std::string());
		void DestroyEntity(Entity entity);

		void OnRuntimeStart();
		void OnRuntimeStop();

		void OnSimulationStart();
		void OnSimulationStop();

		void OnUpdateRuntime(Timestep ts);
		void OnUpdateSimulation(Timestep ts, EditorCamera& camera);
		void OnUpdateEditor(Timestep ts, EditorCamera& camera);
		void OnViewportResize(uint32_t width, uint32_t height);

		Entity DuplicateEntity(Entity entity);

		Entity FindEntityByName(std::string_view name);
		Entity GetEntityByUUID(UUID uuid);

		Entity GetPrimaryCameraEntity();

		bool IsRunning() const { return myIsRunning; }
		bool IsPaused() const { return myIsPaused; }

		void SetPaused(bool paused) { myIsPaused = paused; }

		void Step(int frames = 1);

		template<typename... Components>
		auto GetAllEntitiesWith()
		{
			return myRegistry.view<Components...>();
		}
	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		void OnPhysics2DStart();
		void OnPhysics2DStop();

		void OnPhysics3DStart();
		void OnPhysics3DStop();

		void RenderScene(EditorCamera& camera);
	private:
		entt::registry myRegistry;
		uint32_t myViewportWidth = 0, myViewportHeight = 0;
		bool myIsRunning = false;
		bool myIsPaused = false;
		int myStepFrames = 0;

		b2World*         myPhysicsWorld   = nullptr;
		Physics3DWorld*  myPhysicsWorld3D = nullptr;

		std::unordered_map<UUID, entt::entity> myEntityMap;

		friend class Entity;
		friend class SceneSerializer;
		friend class SceneHierarchyPanel;
	};

}
