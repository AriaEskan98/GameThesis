#include "gepch.h"
#include "GameEngine/Physics/Physics3D.h"

#include <algorithm>
#include <cmath>

#include <glm/glm.hpp>

namespace GameEngine {

	// -----------------------------------------------------------------------
	// Construction / destruction
	// -----------------------------------------------------------------------

	Physics3DWorld::Physics3DWorld()  = default;
	Physics3DWorld::~Physics3DWorld()
	{
		for (Physics3DBody* b : myBodies)
			delete b;
		myBodies.clear();
	}

	// -----------------------------------------------------------------------
	// Body management
	// -----------------------------------------------------------------------

	Physics3DBody* Physics3DWorld::CreateBody(const Physics3DBodyDef& def)
	{
		auto* body = new Physics3DBody();
		body->Position    = def.Position;
		body->Velocity    = { 0.0f, 0.0f, 0.0f };
		body->HalfExtents = def.HalfExtents;
		body->Friction    = def.Friction;
		body->Restitution = def.Restitution;
		body->UseGravity  = def.UseGravity;
		body->IsKinematic = def.IsKinematic;
		body->IsGrounded  = false;
		// Infinite inverse mass (0) = static / immovable body.
		body->InvMass = (def.Mass > 0.0f) ? (1.0f / def.Mass) : 0.0f;

		myBodies.push_back(body);
		return body;
	}

	void Physics3DWorld::DestroyBody(Physics3DBody* body)
	{
		auto it = std::find(myBodies.begin(), myBodies.end(), body);
		if (it != myBodies.end())
		{
			myBodies.erase(it);
			delete body;
		}
	}

	// -----------------------------------------------------------------------
	// Simulation step
	// -----------------------------------------------------------------------

	void Physics3DWorld::Step(float deltaTime)
	{
		const float subDt = deltaTime / static_cast<float>(SubSteps);
		for (int s = 0; s < SubSteps; ++s)
		{
			IntegrateBodies(subDt);
			ResolveCollisions();
		}
	}

	// -----------------------------------------------------------------------
	// Integration  —  semi-implicit Euler
	// -----------------------------------------------------------------------

	void Physics3DWorld::IntegrateBodies(float dt)
	{
		for (Physics3DBody* body : myBodies)
		{
			// Static bodies never move.
			if (body->InvMass == 0.0f)
				continue;

			// Clear grounded flag; it is set by collision resolution.
			body->IsGrounded = false;

			// Apply gravity to dynamic (non-kinematic) bodies.
			if (!body->IsKinematic && body->UseGravity)
				body->Velocity.y += Gravity * dt;

			// Integrate position.
			body->Position += body->Velocity * dt;
		}
	}

	// -----------------------------------------------------------------------
	// Broad + narrow phase — O(n²), fine for game-scale scenes
	// -----------------------------------------------------------------------

	void Physics3DWorld::ResolveCollisions()
	{
		for (size_t i = 0; i < myBodies.size(); ++i)
		{
			for (size_t j = i + 1; j < myBodies.size(); ++j)
			{
				Physics3DBody* a = myBodies[i];
				Physics3DBody* b = myBodies[j];

				// Two static bodies can never collide.
				if (a->InvMass == 0.0f && b->InvMass == 0.0f)
					continue;

				glm::vec3 normal;
				float     depth;
				if (ComputeContact(*a, *b, normal, depth))
					ApplyImpulse(*a, *b, normal, depth);
			}
		}
	}

	// -----------------------------------------------------------------------
	// AABB vs AABB contact using the Separating Axis Theorem
	// -----------------------------------------------------------------------

	bool Physics3DWorld::ComputeContact(const Physics3DBody& a, const Physics3DBody& b,
	                                    glm::vec3& outNormal, float& outDepth)
	{
		const glm::vec3 aMin = a.Position - a.HalfExtents;
		const glm::vec3 aMax = a.Position + a.HalfExtents;
		const glm::vec3 bMin = b.Position - b.HalfExtents;
		const glm::vec3 bMax = b.Position + b.HalfExtents;

		// Overlap on each axis.
		const float dx = std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x);
		const float dy = std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y);
		const float dz = std::min(aMax.z, bMax.z) - std::max(aMin.z, bMin.z);

		if (dx <= 0.0f || dy <= 0.0f || dz <= 0.0f)
			return false; // Separated on at least one axis.

		// Choose the axis with minimum penetration (Minimum Translation Vector).
		const glm::vec3 delta = b.Position - a.Position;

		if (dx <= dy && dx <= dz)
		{
			outNormal = { (delta.x < 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f };
			outDepth  = dx;
		}
		else if (dy <= dx && dy <= dz)
		{
			outNormal = { 0.0f, (delta.y < 0.0f) ? 1.0f : -1.0f, 0.0f };
			outDepth  = dy;
		}
		else
		{
			outNormal = { 0.0f, 0.0f, (delta.z < 0.0f) ? 1.0f : -1.0f };
			outDepth  = dz;
		}

		return true;
	}

	// -----------------------------------------------------------------------
	// Impulse-based collision response
	// -----------------------------------------------------------------------

	void Physics3DWorld::ApplyImpulse(Physics3DBody& a, Physics3DBody& b,
	                                   const glm::vec3& normal, float depth)
	{
		const float totalInvMass = a.InvMass + b.InvMass;
		if (totalInvMass == 0.0f)
			return;

		// --- Positional correction -------------------------------------------
		// Push bodies apart proportional to their inverse mass, with a small
		// slop to avoid jitter from floating-point rounding.
		constexpr float Slop      = 0.005f; // metres; penetration below this is ignored
		constexpr float Percent   = 0.8f;   // fraction of penetration corrected per step

		const float correctionMag = std::max(depth - Slop, 0.0f) * Percent / totalInvMass;
		const glm::vec3 correction = normal * correctionMag;

		a.Position -= correction * a.InvMass;
		b.Position += correction * b.InvMass;

		// --- Velocity impulse ------------------------------------------------
		const glm::vec3 relVel       = b.Velocity - a.Velocity;
		const float     velAlongNorm = glm::dot(relVel, normal);

		// Already separating — only position correction was needed.
		if (velAlongNorm > 0.0f)
		{
			// Still update grounded state even when velocities are separating.
			if (normal.y > 0.707f)  a.IsGrounded = true;
			if (normal.y < -0.707f) b.IsGrounded = true;
			return;
		}

		const float restitution  = std::min(a.Restitution, b.Restitution);
		const float impulseMag   = -(1.0f + restitution) * velAlongNorm / totalInvMass;
		const glm::vec3 impulse  = impulseMag * normal;

		a.Velocity -= a.InvMass * impulse;
		b.Velocity += b.InvMass * impulse;

		// --- Friction impulse (Coulomb model) --------------------------------
		glm::vec3 tangent = relVel - velAlongNorm * normal;
		if (glm::length(tangent) > 1e-4f)
		{
			tangent = glm::normalize(tangent);
			float frictionMag = -glm::dot(relVel, tangent) / totalInvMass;

			// Clamp to the friction cone.
			const float mu = std::sqrt(a.Friction * b.Friction);
			frictionMag = glm::clamp(frictionMag, -impulseMag * mu, impulseMag * mu);

			const glm::vec3 frictionImpulse = frictionMag * tangent;
			a.Velocity -= a.InvMass * frictionImpulse;
			b.Velocity += b.InvMass * frictionImpulse;
		}

		// --- Ground detection ------------------------------------------------
		// Contact normal pointing upward → A is resting on B's surface.
		// (threshold cos 45° = 0.707 so diagonal contacts also count)
		if (normal.y > 0.707f)  a.IsGrounded = true;
		if (normal.y < -0.707f) b.IsGrounded = true;
	}

} // namespace GameEngine
