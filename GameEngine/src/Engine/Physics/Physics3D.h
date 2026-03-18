#pragma once

#include <glm/glm.hpp>
#include <vector>

namespace GameEngine {

	// -----------------------------------------------------------------------
	// Physics3DBody
	// -----------------------------------------------------------------------

	/// All mutable state for one rigid body inside the Physics3DWorld.
	/// Pointers to these objects are stored in Rigidbody3DComponent::RuntimeBody
	/// so that the Scene can sync results back to the ECS each frame.
	struct Physics3DBody
	{
		glm::vec3 Position  = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Velocity  = { 0.0f, 0.0f, 0.0f };

		/// Reciprocal of mass.  0 = infinite mass (static / immovable body).
		float InvMass = 1.0f;

		/// AABB half-extents in local (unrotated) space.
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };

		float Friction    = 0.5f;
		float Restitution = 0.0f;

		bool UseGravity    = true;
		bool IsKinematic   = false; ///< Velocity-driven; forces are not applied (use for players).
		bool IsGrounded    = false; ///< Set by the solver when resting on a surface.
	};

	// -----------------------------------------------------------------------
	// Physics3DBodyDef  —  creation parameters
	// -----------------------------------------------------------------------

	struct Physics3DBodyDef
	{
		glm::vec3 Position    = { 0.0f, 0.0f, 0.0f };
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };
		float Mass            = 1.0f;   ///< 0 → static body.
		float Friction        = 0.5f;
		float Restitution     = 0.0f;
		bool  UseGravity      = true;
		bool  IsKinematic     = false;
	};

	// -----------------------------------------------------------------------
	// Physics3DWorld
	// -----------------------------------------------------------------------

	/// Simple AABB-based rigid-body physics world suitable for an FPS game.
	///
	/// Features:
	///  - Gravity integration (semi-implicit Euler).
	///  - AABB vs AABB collision detection and resolution with position
	///    correction and velocity impulse (restitution + friction).
	///  - Per-body "IsGrounded" flag computed from contact normals.
	///
	/// Design notes:
	///  - Static bodies have Mass == 0 (InvMass == 0) and are never moved.
	///  - Kinematic bodies are moved by setting their Velocity externally;
	///    gravity is not applied to them (used for the player character).
	///  - The solver runs multiple sub-steps per frame for stability.
	class Physics3DWorld
	{
	public:
		Physics3DWorld();
		~Physics3DWorld();

		/// Advance the simulation by deltaTime seconds.
		void Step(float deltaTime);

		/// Allocate and register a new body. The caller must call DestroyBody
		/// before deleting the world.
		Physics3DBody* CreateBody(const Physics3DBodyDef& def);

		/// Unregister and free a body created by CreateBody.
		void DestroyBody(Physics3DBody* body);

		// ---- Configuration -----------------------------------------------
		static constexpr float Gravity     = -9.81f; ///< m/s²
		static constexpr int   SubSteps    = 3;       ///< Collision solver iterations per Step.

	private:
		void IntegrateBodies(float dt);
		void ResolveCollisions();

		/// Returns true when the two AABBs overlap and writes the
		/// contact normal (pointing from B toward A) and penetration depth.
		static bool ComputeContact(const Physics3DBody& a, const Physics3DBody& b,
		                           glm::vec3& outNormal, float& outDepth);

		/// Apply position correction and velocity impulse to resolve one contact.
		static void ApplyImpulse(Physics3DBody& a, Physics3DBody& b,
		                         const glm::vec3& normal, float depth);

		std::vector<Physics3DBody*> myBodies;
	};

} // namespace GameEngine
