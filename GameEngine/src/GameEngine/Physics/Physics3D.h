#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace GameEngine {

	// -----------------------------------------------------------------------
	// Physics3DBody
	// -----------------------------------------------------------------------

	/// State synced back from PhysX each frame.
	/// Pointers to these objects are stored in Rigidbody3DComponent::RuntimeBody
	/// so that the Scene can read results back to the ECS each frame.
	struct Physics3DBody
	{
		glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Velocity  = { 0.0f, 0.0f, 0.0f };
		bool      IsGrounded = false;

		/// Opaque pointer to the underlying physx::PxRigidActor.
		/// Do not touch this outside Physics3D.cpp.
		void* Actor = nullptr;
	};

	// -----------------------------------------------------------------------
	// Physics3DBodyDef  —  creation parameters
	// -----------------------------------------------------------------------

	struct Physics3DBodyDef
	{
		glm::vec3 Position    = { 0.0f, 0.0f, 0.0f };
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };
		float Mass        = 1.0f;   ///< 0 → static body.
		float Friction    = 0.5f;
		float Restitution = 0.0f;
		bool  UseGravity  = true;
		bool  IsKinematic = false;
	};

	// -----------------------------------------------------------------------
	// Physics3DWorld
	// -----------------------------------------------------------------------

	/// PhysX-backed rigid-body physics world.
	///
	/// Wraps a PxScene and exposes the same interface as the old custom solver
	/// so that Scene.cpp needs no changes.
	class Physics3DWorld
	{
	public:
		Physics3DWorld();
		~Physics3DWorld();

		/// Advance the simulation by deltaTime seconds.
		void Step(float deltaTime);

		/// Allocate and register a new body.
		Physics3DBody* CreateBody(const Physics3DBodyDef& def);

		/// Unregister and free a body created by CreateBody.
		void DestroyBody(Physics3DBody* body);

	private:
		struct PhysXImpl;
		PhysXImpl* myImpl = nullptr;

		std::vector<Physics3DBody*> myBodies;
	};

} // namespace GameEngine
