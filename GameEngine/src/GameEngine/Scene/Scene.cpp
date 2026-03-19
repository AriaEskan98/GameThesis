#include "gepch.h"
#include "Scene.h"
#include "Entity.h"

#include "Components.h"
#include "ScriptableEntity.h"
#include "GameEngine/Renderer/Renderer3D.h"
#include "GameEngine/Physics/Physics3D.h"

#include <glm/glm.hpp>

#include "Entity.h"

namespace GameEngine {

	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		delete myPhysicsWorld3D;
	}

	template<typename... Component>
	static void CopyComponent(entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		([&]()
		{
			auto view = src.view<Component>();
			for (auto srcEntity : view)
			{
				entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

				auto& srcComponent = src.get<Component>(srcEntity);
				dst.emplace_or_replace<Component>(dstEntity, srcComponent);
			}
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponent(ComponentGroup<Component...>, entt::registry& dst, entt::registry& src, const std::unordered_map<UUID, entt::entity>& enttMap)
	{
		CopyComponent<Component...>(dst, src, enttMap);
	}

	template<typename... Component>
	static void CopyComponentIfExists(Entity dst, Entity src)
	{
		([&]()
		{
			if (src.HasComponent<Component>())
				dst.AddOrReplaceComponent<Component>(src.GetComponent<Component>());
		}(), ...);
	}

	template<typename... Component>
	static void CopyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src)
	{
		CopyComponentIfExists<Component...>(dst, src);
	}

	Handle<Scene> Scene::Copy(Handle<Scene> other)
	{
		Handle<Scene> newScene = MakeHandle<Scene>();

		newScene->myViewportWidth = other->myViewportWidth;
		newScene->myViewportHeight = other->myViewportHeight;

		auto& srcSceneRegistry = other->myRegistry;
		auto& dstSceneRegistry = newScene->myRegistry;
		std::unordered_map<UUID, entt::entity> enttMap;

		// Create entities in new scene
		auto idView = srcSceneRegistry.view<IDComponent>();
		for (auto e : idView)
		{
			UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
			const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
			Entity newEntity = newScene->CreateEntityWithUUID(uuid, name);
			enttMap[uuid] = (entt::entity)newEntity;
		}

		// Copy components (except IDComponent and TagComponent)
		CopyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

		return newScene;
	}

	Entity Scene::CreateEntity(const std::string& name)
	{
		return CreateEntityWithUUID(UUID(), name);
	}

	Entity Scene::CreateEntityWithUUID(UUID uuid, const std::string& name)
	{
		Entity entity = { myRegistry.create(), this };
		entity.AddComponent<IDComponent>(uuid);
		entity.AddComponent<TransformComponent>();
		auto& tag = entity.AddComponent<TagComponent>();
		tag.Tag = name.empty() ? "Entity" : name;

		myEntityMap[uuid] = entity;

		return entity;
	}

	void Scene::DestroyEntity(Entity entity)
	{
		myEntityMap.erase(entity.GetUUID());
		myRegistry.destroy(entity);
	}

	void Scene::OnRuntimeStart()
	{
		myIsRunning = true;

		OnPhysics3DStart();

		// Instantiate native scripts
		myRegistry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (!nsc.Instance)
			{
				nsc.Instance = nsc.InstantiateScript();
				nsc.Instance->myEntity = Entity{ entity, this };
				nsc.Instance->OnCreate();
			}
		});
	}

	void Scene::OnRuntimeStop()
	{
		myIsRunning = false;

		OnPhysics3DStop();

		// Destroy native scripts
		myRegistry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
		{
			if (nsc.Instance)
				nsc.DestroyScript(&nsc);
		});
	}

	void Scene::OnSimulationStart()
	{
		OnPhysics3DStart();
	}

	void Scene::OnSimulationStop()
	{
		OnPhysics3DStop();
	}

	void Scene::OnUpdateRuntime(Timestep ts)
	{
		if (!myIsPaused || myStepFrames-- > 0)
		{
			// Update native scripts
			myRegistry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
			{
				if (!nsc.Instance)
				{
					nsc.Instance = nsc.InstantiateScript();
					nsc.Instance->myEntity = Entity{ entity, this };
					nsc.Instance->OnCreate();
				}

				nsc.Instance->OnUpdate(ts);
			});

			// 3D Physics
			{
				myPhysicsWorld3D->Step(ts);

				auto view3d = myRegistry.view<Rigidbody3DComponent>();
				for (auto e : view3d)
				{
					Entity entity = { e, this };
					auto& transform = entity.GetComponent<TransformComponent>();
					auto& rb3d      = entity.GetComponent<Rigidbody3DComponent>();

					if (rb3d.Type == Rigidbody3DComponent::BodyType::Static)
						continue;

					auto* body = (Physics3DBody*)rb3d.RuntimeBody;
					transform.Translation = body->Position;
				}
			}
		}

		// Find primary camera
		Camera* mainCamera = nullptr;
		glm::mat4 cameraTransform;
		{
			auto view = myRegistry.view<TransformComponent, CameraComponent>();
			for (auto entity : view)
			{
				auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

				if (camera.Primary)
				{
					mainCamera = &camera.Camera;
					cameraTransform = transform.GetTransform();
					break;
				}
			}
		}

		if (mainCamera)
		{
			Renderer3D::BeginScene(*mainCamera, cameraTransform);
			RenderMeshes();
			Renderer3D::EndScene();
		}
	}

	void Scene::OnUpdateSimulation(Timestep ts, EditorCamera& camera)
	{
		if (!myIsPaused || myStepFrames-- > 0)
		{
			// 3D Physics
			{
				myPhysicsWorld3D->Step(ts);

				auto view3d = myRegistry.view<Rigidbody3DComponent>();
				for (auto e : view3d)
				{
					Entity entity = { e, this };
					auto& transform = entity.GetComponent<TransformComponent>();
					auto& rb3d      = entity.GetComponent<Rigidbody3DComponent>();

					if (rb3d.Type == Rigidbody3DComponent::BodyType::Static)
						continue;

					auto* body = (Physics3DBody*)rb3d.RuntimeBody;
					transform.Translation = body->Position;
				}
			}
		}

		RenderScene(camera);
	}

	void Scene::OnUpdateEditor(Timestep ts, EditorCamera& camera)
	{
		RenderScene(camera);
	}

	void Scene::OnViewportResize(uint32_t width, uint32_t height)
	{
		if (myViewportWidth == width && myViewportHeight == height)
			return;

		myViewportWidth = width;
		myViewportHeight = height;

		// Resize our non-FixedAspectRatio cameras
		auto view = myRegistry.view<CameraComponent>();
		for (auto entity : view)
		{
			auto& cameraComponent = view.get<CameraComponent>(entity);
			if (!cameraComponent.FixedAspectRatio)
				cameraComponent.Camera.SetViewportSize(width, height);
		}
	}

	Entity Scene::GetPrimaryCameraEntity()
	{
		auto view = myRegistry.view<CameraComponent>();
		for (auto entity : view)
		{
			const auto& camera = view.get<CameraComponent>(entity);
			if (camera.Primary)
				return Entity{entity, this};
		}
		return {};
	}

	void Scene::Step(int frames)
	{
		myStepFrames = frames;
	}

	Entity Scene::DuplicateEntity(Entity entity)
	{
		std::string name = entity.GetName();
		Entity newEntity = CreateEntity(name);
		CopyComponentIfExists(AllComponents{}, newEntity, entity);
		return newEntity;
	}

	Entity Scene::FindEntityByName(std::string_view name)
	{
		auto view = myRegistry.view<TagComponent>();
		for (auto entity : view)
		{
			const TagComponent& tc = view.get<TagComponent>(entity);
			if (tc.Tag == name)
				return Entity{ entity, this };
		}
		return {};
	}

	Entity Scene::GetEntityByUUID(UUID uuid)
	{
		if (myEntityMap.find(uuid) != myEntityMap.end())
			return { myEntityMap.at(uuid), this };

		return {};
	}

	void Scene::OnPhysics3DStart()
	{
		myPhysicsWorld3D = new Physics3DWorld();

		auto view = myRegistry.view<Rigidbody3DComponent>();
		for (auto e : view)
		{
			Entity entity = { e, this };
			auto& transform = entity.GetComponent<TransformComponent>();
			auto& rb3d      = entity.GetComponent<Rigidbody3DComponent>();

			Physics3DBodyDef def;
			def.Position    = transform.Translation;
			def.Friction    = rb3d.Friction;
			def.Restitution = rb3d.Restitution;
			def.UseGravity  = rb3d.UseGravity;
			def.IsKinematic = (rb3d.Type == Rigidbody3DComponent::BodyType::Kinematic);
			def.Mass = (rb3d.Type == Rigidbody3DComponent::BodyType::Static) ? 0.0f : rb3d.Mass;

			if (entity.HasComponent<BoxCollider3DComponent>())
			{
				auto& bc3d      = entity.GetComponent<BoxCollider3DComponent>();
				def.HalfExtents = bc3d.HalfExtents * transform.Scale;
				def.Position   += bc3d.Offset;
			}
			else
			{
				def.HalfExtents = transform.Scale * 0.5f;
			}

			rb3d.RuntimeBody = myPhysicsWorld3D->CreateBody(def);
		}
	}

	void Scene::OnPhysics3DStop()
	{
		delete myPhysicsWorld3D;
		myPhysicsWorld3D = nullptr;
	}

	void Scene::RenderMeshes()
	{
		LightEnvironment lights;

		// Directional light — only the first entity wins.
		{
			auto view = myRegistry.view<TransformComponent, DirectionalLightComponent>();
			for (auto entity : view)
			{
				auto [transform, light] = view.get<TransformComponent, DirectionalLightComponent>(entity);
				glm::vec3 dir = glm::normalize(
					glm::quat(transform.Rotation) * glm::vec3(0.0f, -1.0f, 0.0f));
				lights.DirectionalLight = { dir, light.Color, light.Intensity };
				lights.HasDirectionalLight = true;
				break;
			}
		}

		// Point lights — up to Renderer3D::MaxPointLights.
		{
			auto view = myRegistry.view<TransformComponent, PointLightComponent>();
			for (auto entity : view)
			{
				if ((int)lights.PointLights.size() >= Renderer3D::MaxPointLights)
					break;
				auto [transform, light] = view.get<TransformComponent, PointLightComponent>(entity);
				lights.PointLights.push_back({
					transform.Translation,
					light.Color, light.Intensity,
					light.Constant, light.Linear, light.Quadratic
				});
			}
		}

		Renderer3D::SetLightEnvironment(lights);

		auto view = myRegistry.view<TransformComponent, MeshRendererComponent>();
		for (auto entity : view)
		{
			auto [transform, mesh] = view.get<TransformComponent, MeshRendererComponent>(entity);
			if (mesh.Mesh)
				Renderer3D::Submit(mesh.Mesh, transform.GetTransform(), mesh.Color, (int)entity);
		}
	}

	void Scene::RenderScene(EditorCamera& camera)
	{
		Renderer3D::BeginScene(camera);
		RenderMeshes();
		Renderer3D::EndScene();
	}

	template<typename T>
	void Scene::OnComponentAdded(Entity entity, T& component)
	{
		static_assert(sizeof(T) == 0);
	}

	template<>
	void Scene::OnComponentAdded<IDComponent>(Entity entity, IDComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
	{
		if (myViewportWidth > 0 && myViewportHeight > 0)
			component.Camera.SetViewportSize(myViewportWidth, myViewportHeight);
	}

	template<>
	void Scene::OnComponentAdded<TagComponent>(Entity entity, TagComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<MeshRendererComponent>(Entity entity, MeshRendererComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<DirectionalLightComponent>(Entity entity, DirectionalLightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<PointLightComponent>(Entity entity, PointLightComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<Rigidbody3DComponent>(Entity entity, Rigidbody3DComponent& component)
	{
	}

	template<>
	void Scene::OnComponentAdded<BoxCollider3DComponent>(Entity entity, BoxCollider3DComponent& component)
	{
	}

}
