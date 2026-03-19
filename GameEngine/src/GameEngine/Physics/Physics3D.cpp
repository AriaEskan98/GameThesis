#include "gepch.h"
#include "Physics3D.h"

#include <PxPhysicsAPI.h>

#include <algorithm>

using namespace physx;

namespace GameEngine {

	// -----------------------------------------------------------------------
	// PhysXImpl  —  owns all PhysX SDK objects for one world
	// -----------------------------------------------------------------------

	struct Physics3DWorld::PhysXImpl
	{
		PxDefaultAllocator      Allocator;
		PxDefaultErrorCallback  ErrorCallback;

		PxFoundation*            Foundation  = nullptr;
		PxPhysics*               Physics     = nullptr;
		PxDefaultCpuDispatcher*  Dispatcher  = nullptr;
		PxScene*                 Scene       = nullptr;
	};

	// -----------------------------------------------------------------------
	// Construction / destruction
	// -----------------------------------------------------------------------

	Physics3DWorld::Physics3DWorld()
	{
		myImpl = new PhysXImpl();

		myImpl->Foundation = PxCreateFoundation(PX_PHYSICS_VERSION,
			myImpl->Allocator, myImpl->ErrorCallback);
		GE_CORE_ASSERT(myImpl->Foundation, "PxCreateFoundation failed");

		myImpl->Physics = PxCreatePhysics(PX_PHYSICS_VERSION, *myImpl->Foundation,
			PxTolerancesScale(), false, nullptr);
		GE_CORE_ASSERT(myImpl->Physics, "PxCreatePhysics failed");

		PxSceneDesc desc(myImpl->Physics->getTolerancesScale());
		desc.gravity       = PxVec3(0.0f, -9.81f, 0.0f);
		myImpl->Dispatcher = PxDefaultCpuDispatcherCreate(1);
		desc.cpuDispatcher = myImpl->Dispatcher;
		desc.filterShader  = PxDefaultSimulationFilterShader;

		myImpl->Scene = myImpl->Physics->createScene(desc);
		GE_CORE_ASSERT(myImpl->Scene, "PxPhysics::createScene failed");
	}

	Physics3DWorld::~Physics3DWorld()
	{
		for (auto* body : myBodies)
		{
			if (body->Actor)
			{
				auto* actor = static_cast<PxRigidActor*>(body->Actor);
				myImpl->Scene->removeActor(*actor);
				actor->release();
			}
			delete body;
		}
		myBodies.clear();

		myImpl->Scene->release();
		myImpl->Dispatcher->release();
		myImpl->Physics->release();
		myImpl->Foundation->release();
		delete myImpl;
	}

	// -----------------------------------------------------------------------
	// Body management
	// -----------------------------------------------------------------------

	Physics3DBody* Physics3DWorld::CreateBody(const Physics3DBodyDef& def)
	{
		auto* body     = new Physics3DBody();
		body->Position = def.Position;

		const PxTransform  pose(PxVec3(def.Position.x, def.Position.y, def.Position.z));
		const PxBoxGeometry geometry(def.HalfExtents.x, def.HalfExtents.y, def.HalfExtents.z);

		// One material per body using the supplied friction/restitution values.
		PxMaterial* mat = myImpl->Physics->createMaterial(
			def.Friction, def.Friction, def.Restitution);

		if (def.Mass <= 0.0f)
		{
			// --- Static body ------------------------------------------------
			PxRigidStatic* actor = myImpl->Physics->createRigidStatic(pose);
			PxShape* shape = myImpl->Physics->createShape(geometry, *mat);
			actor->attachShape(*shape);
			shape->release();
			myImpl->Scene->addActor(*actor);
			body->Actor = actor;
		}
		else
		{
			// --- Dynamic / Kinematic body ------------------------------------
			PxRigidDynamic* actor = myImpl->Physics->createRigidDynamic(pose);
			PxShape* shape = myImpl->Physics->createShape(geometry, *mat);
			actor->attachShape(*shape);
			shape->release();

			PxRigidBodyExt::setMassAndUpdateInertia(*actor, def.Mass);

			if (def.IsKinematic)
				actor->setRigidBodyFlag(PxRigidBodyFlag::eKINEMATIC, true);

			if (!def.UseGravity)
				actor->setActorFlag(PxActorFlag::eDISABLE_GRAVITY, true);

			myImpl->Scene->addActor(*actor);
			body->Actor = actor;
		}

		mat->release();
		myBodies.push_back(body);
		return body;
	}

	void Physics3DWorld::DestroyBody(Physics3DBody* body)
	{
		auto it = std::find(myBodies.begin(), myBodies.end(), body);
		if (it == myBodies.end())
			return;

		myBodies.erase(it);

		auto* actor = static_cast<PxRigidActor*>(body->Actor);
		myImpl->Scene->removeActor(*actor);
		actor->release();
		delete body;
	}

	// -----------------------------------------------------------------------
	// Simulation step
	// -----------------------------------------------------------------------

	void Physics3DWorld::Step(float deltaTime)
	{
		if (deltaTime <= 0.0f)
			return;

		myImpl->Scene->simulate(deltaTime);
		myImpl->Scene->fetchResults(true);

		// Sync transform and velocity back to our body structs.
		for (auto* body : myBodies)
		{
			auto* actor   = static_cast<PxRigidActor*>(body->Actor);
			auto* dynamic = actor->is<PxRigidDynamic>();

			// Static and kinematic actors don't move — nothing to sync.
			if (!dynamic || dynamic->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC))
				continue;

			const PxTransform pose = dynamic->getGlobalPose();
			body->Position = { pose.p.x, pose.p.y, pose.p.z };

			const PxVec3 vel = dynamic->getLinearVelocity();
			body->Velocity = { vel.x, vel.y, vel.z };

			// Grounded: a very small downward velocity means resting on a surface.
			body->IsGrounded = (body->Velocity.y > -0.1f && body->Velocity.y < 0.1f);
		}
	}

} // namespace GameEngine
