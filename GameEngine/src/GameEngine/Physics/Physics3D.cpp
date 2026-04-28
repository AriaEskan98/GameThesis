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

		// Propagate the foundation singleton to PhysXFoundation_64.dll so that
		// PhysXCommon_64.dll can find it via PxGetFoundation(). Without this,
		// PxCreateFoundation stores the pointer in the static-lib context (EXE)
		// while cooking code running inside PhysXCommon_64.dll reads a separate
		// null singleton from PhysXFoundation_64.dll.
		PxSetFoundationInstance(*myImpl->Foundation);

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
			delete body;  // Actor == nullptr is safe; body was cooked but had no actor
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

		const PxQuat pxRot(def.Rotation.x, def.Rotation.y, def.Rotation.z, def.Rotation.w);
		const PxTransform  pose(PxVec3(def.Position.x, def.Position.y, def.Position.z), pxRot);
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

	Physics3DBody* Physics3DWorld::CreateTriMeshBody(const Physics3DTriMeshDef& def)
	{
		GE_CORE_ASSERT(!def.Vertices.empty() && !def.Indices.empty(),
			"CreateTriMeshBody: empty mesh data");

		auto* body     = new Physics3DBody();
		body->Position = def.Position;

		// Convert glm vertices to PxVec3
		std::vector<PxVec3> pxVerts;
		pxVerts.reserve(def.Vertices.size());
		for (const auto& v : def.Vertices)
			pxVerts.push_back({ v.x, v.y, v.z });

		PxTriangleMeshDesc meshDesc;
		meshDesc.points.count  = (PxU32)pxVerts.size();
		meshDesc.points.stride = sizeof(PxVec3);
		meshDesc.points.data   = pxVerts.data();
		meshDesc.triangles.count  = (PxU32)(def.Indices.size() / 3);
		meshDesc.triangles.stride = 3 * sizeof(PxU32);
		meshDesc.triangles.data   = def.Indices.data();

		// Use PxCreateTriangleMesh (PhysX 5.x API) — inserts directly via the
		// physics insertion callback, skipping binary serialisation entirely.
		// Weld vertices at 1 mm tolerance to fix 3ds Max duplicate-vert exports.
		PxCookingParams cookParams(myImpl->Physics->getTolerancesScale());
		cookParams.meshPreprocessParams = PxMeshPreprocessingFlag::eWELD_VERTICES;
		cookParams.meshWeldTolerance    = 0.001f;

		PxTriangleMesh* triMesh = PxCreateTriangleMesh(
			cookParams, meshDesc,
			myImpl->Physics->getPhysicsInsertionCallback());
		if (!triMesh)
		{
			GE_CORE_ERROR("CreateTriMeshBody: PxCreateTriangleMesh failed — skipping actor");
			myBodies.push_back(body);
			return body;
		}

		const PxQuat pxRot(def.Rotation.x, def.Rotation.y, def.Rotation.z, def.Rotation.w);
		const PxTransform pose(PxVec3(def.Position.x, def.Position.y, def.Position.z), pxRot);

		PxMaterial* mat = myImpl->Physics->createMaterial(def.Friction, def.Friction, def.Restitution);

		PxTriangleMeshGeometry geom(triMesh);
		PxRigidStatic* actor = myImpl->Physics->createRigidStatic(pose);
		PxShape* shape = myImpl->Physics->createShape(geom, *mat);
		actor->attachShape(*shape);
		shape->release();
		mat->release();
		triMesh->release(); // PxShape holds its own reference

		myImpl->Scene->addActor(*actor);
		body->Actor = actor;
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

		// Apply any externally-set velocity (e.g. from FPSCameraController) before simulating.
		// For physics-only bodies (crate, etc.) this is a no-op: the value we write back
		// equals what PhysX computed last frame, so nothing changes.
		for (auto* body : myBodies)
		{
			if (!body->Actor) continue;
			auto* actor   = static_cast<PxRigidActor*>(body->Actor);
			auto* dynamic = actor->is<PxRigidDynamic>();
			if (!dynamic || dynamic->getRigidBodyFlags().isSet(PxRigidBodyFlag::eKINEMATIC))
				continue;
			dynamic->setLinearVelocity(PxVec3(body->Velocity.x, body->Velocity.y, body->Velocity.z));
		}

		myImpl->Scene->simulate(deltaTime);
		myImpl->Scene->fetchResults(true);

		// Sync transform and velocity back to our body structs.
		for (auto* body : myBodies)
		{
			if (!body->Actor) continue;
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
