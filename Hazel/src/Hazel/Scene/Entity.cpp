#include "gepch.h"
#include "Entity.h"

namespace GameEngine {

	Entity::Entity(entt::entity handle, Scene* scene)
		: myEntityHandle(handle), myScene(scene)
	{
	}

}