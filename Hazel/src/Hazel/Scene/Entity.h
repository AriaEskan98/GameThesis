#pragma once

#include "Hazel/Core/UUID.h"
#include "Scene.h"
#include "Components.h"

#include "entt.hpp"

namespace GameEngine {

	class Entity
	{
	public:
		Entity() = default;
		Entity(entt::entity handle, Scene* scene);
		Entity(const Entity& other) = default;

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args)
		{
			GE_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
			T& component = myScene->myRegistry.emplace<T>(myEntityHandle, std::forward<Args>(args)...);
			myScene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T, typename... Args>
		T& AddOrReplaceComponent(Args&&... args)
		{
			T& component = myScene->myRegistry.emplace_or_replace<T>(myEntityHandle, std::forward<Args>(args)...);
			myScene->OnComponentAdded<T>(*this, component);
			return component;
		}

		template<typename T>
		T& GetComponent()
		{
			GE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			return myScene->myRegistry.get<T>(myEntityHandle);
		}

		template<typename T>
		bool HasComponent()
		{
			return myScene->myRegistry.has<T>(myEntityHandle);
		}

		template<typename T>
		void RemoveComponent()
		{
			GE_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
			myScene->myRegistry.remove<T>(myEntityHandle);
		}

		operator bool() const { return myEntityHandle != entt::null; }
		operator entt::entity() const { return myEntityHandle; }
		operator uint32_t() const { return (uint32_t)myEntityHandle; }

		UUID GetUUID() { return GetComponent<IDComponent>().ID; }
		const std::string& GetName() { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& other) const
		{
			return myEntityHandle == other.myEntityHandle && myScene == other.myScene;
		}

		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}
	private:
		entt::entity myEntityHandle{ entt::null };
		Scene* myScene = nullptr;
	};

}
