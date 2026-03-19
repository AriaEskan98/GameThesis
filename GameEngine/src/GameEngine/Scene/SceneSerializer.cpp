#include "gepch.h"
#include "SceneSerializer.h"

#include "Entity.h"
#include "Components.h"
#include "GameEngine/Core/UUID.h"

#include "GameEngine/Project/Project.h"

#include <fstream>

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<GameEngine::UUID>
	{
		static Node encode(const GameEngine::UUID& uuid)
		{
			Node node;
			node.push_back((uint64_t)uuid);
			return node;
		}

		static bool decode(const Node& node, GameEngine::UUID& uuid)
		{
			uuid = node.as<uint64_t>();
			return true;
		}
	};

}

namespace GameEngine {

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	static std::string RigidBody3DBodyTypeToString(Rigidbody3DComponent::BodyType bodyType)
	{
		switch (bodyType)
		{
			case Rigidbody3DComponent::BodyType::Static:    return "Static";
			case Rigidbody3DComponent::BodyType::Dynamic:   return "Dynamic";
			case Rigidbody3DComponent::BodyType::Kinematic: return "Kinematic";
		}

		GE_CORE_ASSERT(false, "Unknown body type");
		return {};
	}

	static Rigidbody3DComponent::BodyType RigidBody3DBodyTypeFromString(const std::string& bodyTypeString)
	{
		if (bodyTypeString == "Static")    return Rigidbody3DComponent::BodyType::Static;
		if (bodyTypeString == "Dynamic")   return Rigidbody3DComponent::BodyType::Dynamic;
		if (bodyTypeString == "Kinematic") return Rigidbody3DComponent::BodyType::Kinematic;

		GE_CORE_ASSERT(false, "Unknown body type");
		return Rigidbody3DComponent::BodyType::Static;
	}

	SceneSerializer::SceneSerializer(const Handle<Scene>& scene)
		: myScene(scene)
	{
	}

	static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		GE_CORE_ASSERT(entity.HasComponent<IDComponent>());

		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetUUID();

		if (entity.HasComponent<TagComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap;
			out << YAML::Key << "Tag" << YAML::Value << entity.GetComponent<TagComponent>().Tag;
			out << YAML::EndMap;
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap;

			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap;

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap;
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspectiveVerticalFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspectiveNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspectiveFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthographicSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthographicNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthographicFarClip();
			out << YAML::EndMap;

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.Primary;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<MeshRendererComponent>())
		{
			out << YAML::Key << "MeshRendererComponent";
			out << YAML::BeginMap;

			auto& mrc = entity.GetComponent<MeshRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << mrc.Color;
			// TODO: serialize mesh asset path

			out << YAML::EndMap;
		}

		if (entity.HasComponent<DirectionalLightComponent>())
		{
			out << YAML::Key << "DirectionalLightComponent";
			out << YAML::BeginMap;

			auto& dlc = entity.GetComponent<DirectionalLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << dlc.Color;
			out << YAML::Key << "Intensity" << YAML::Value << dlc.Intensity;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<PointLightComponent>())
		{
			out << YAML::Key << "PointLightComponent";
			out << YAML::BeginMap;

			auto& plc = entity.GetComponent<PointLightComponent>();
			out << YAML::Key << "Color" << YAML::Value << plc.Color;
			out << YAML::Key << "Intensity" << YAML::Value << plc.Intensity;
			out << YAML::Key << "Constant" << YAML::Value << plc.Constant;
			out << YAML::Key << "Linear" << YAML::Value << plc.Linear;
			out << YAML::Key << "Quadratic" << YAML::Value << plc.Quadratic;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<Rigidbody3DComponent>())
		{
			out << YAML::Key << "Rigidbody3DComponent";
			out << YAML::BeginMap;

			auto& rb3d = entity.GetComponent<Rigidbody3DComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << RigidBody3DBodyTypeToString(rb3d.Type);
			out << YAML::Key << "Mass" << YAML::Value << rb3d.Mass;
			out << YAML::Key << "Friction" << YAML::Value << rb3d.Friction;
			out << YAML::Key << "Restitution" << YAML::Value << rb3d.Restitution;
			out << YAML::Key << "UseGravity" << YAML::Value << rb3d.UseGravity;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollider3DComponent>())
		{
			out << YAML::Key << "BoxCollider3DComponent";
			out << YAML::BeginMap;

			auto& bc3d = entity.GetComponent<BoxCollider3DComponent>();
			out << YAML::Key << "Offset" << YAML::Value << bc3d.Offset;
			out << YAML::Key << "HalfExtents" << YAML::Value << bc3d.HalfExtents;

			out << YAML::EndMap;
		}

		out << YAML::EndMap; // Entity
	}

	void SceneSerializer::Serialize(const std::string& filepath)
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Scene" << YAML::Value << "Untitled";
		out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
		myScene->myRegistry.each([&](auto entityID)
		{
			Entity entity = { entityID, myScene.get() };
			if (!entity)
				return;

			SerializeEntity(out, entity);
		});
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::ofstream fout(filepath);
		fout << out.c_str();
	}

	void SceneSerializer::SerializeRuntime(const std::string& filepath)
	{
		// Not implemented
		GE_CORE_ASSERT(false);
	}

	bool SceneSerializer::Deserialize(const std::string& filepath)
	{
		YAML::Node data;
		try
		{
			data = YAML::LoadFile(filepath);
		}
		catch (YAML::ParserException e)
		{
			GE_CORE_ERROR("Failed to load scene file '{0}'\n     {1}", filepath, e.what());
			return false;
		}

		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		GE_CORE_TRACE("Deserializing scene '{0}'", sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				GE_CORE_TRACE("Deserialized entity with ID = {0}, name = {1}", uuid, name);

				Entity deserializedEntity = myScene->CreateEntityWithUUID(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Translation = transformComponent["Translation"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();
					auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::ProjectionType)cameraProps["ProjectionType"].as<int>());
					cc.Camera.SetPerspectiveVerticalFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspectiveNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspectiveFarClip(cameraProps["PerspectiveFar"].as<float>());
					cc.Camera.SetOrthographicSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthographicNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthographicFarClip(cameraProps["OrthographicFar"].as<float>());
					cc.Primary = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto meshRendererComponent = entity["MeshRendererComponent"];
				if (meshRendererComponent)
				{
					auto& mrc = deserializedEntity.AddComponent<MeshRendererComponent>();
					mrc.Color = meshRendererComponent["Color"].as<glm::vec4>();
					// TODO: load mesh asset from path
				}

				auto directionalLightComponent = entity["DirectionalLightComponent"];
				if (directionalLightComponent)
				{
					auto& dlc = deserializedEntity.AddComponent<DirectionalLightComponent>();
					dlc.Color = directionalLightComponent["Color"].as<glm::vec3>();
					dlc.Intensity = directionalLightComponent["Intensity"].as<float>();
				}

				auto pointLightComponent = entity["PointLightComponent"];
				if (pointLightComponent)
				{
					auto& plc = deserializedEntity.AddComponent<PointLightComponent>();
					plc.Color = pointLightComponent["Color"].as<glm::vec3>();
					plc.Intensity = pointLightComponent["Intensity"].as<float>();
					plc.Constant = pointLightComponent["Constant"].as<float>();
					plc.Linear = pointLightComponent["Linear"].as<float>();
					plc.Quadratic = pointLightComponent["Quadratic"].as<float>();
				}

				auto rigidbody3DComponent = entity["Rigidbody3DComponent"];
				if (rigidbody3DComponent)
				{
					auto& rb3d = deserializedEntity.AddComponent<Rigidbody3DComponent>();
					rb3d.Type = RigidBody3DBodyTypeFromString(rigidbody3DComponent["BodyType"].as<std::string>());
					rb3d.Mass = rigidbody3DComponent["Mass"].as<float>();
					rb3d.Friction = rigidbody3DComponent["Friction"].as<float>();
					rb3d.Restitution = rigidbody3DComponent["Restitution"].as<float>();
					rb3d.UseGravity = rigidbody3DComponent["UseGravity"].as<bool>();
				}

				auto boxCollider3DComponent = entity["BoxCollider3DComponent"];
				if (boxCollider3DComponent)
				{
					auto& bc3d = deserializedEntity.AddComponent<BoxCollider3DComponent>();
					bc3d.Offset = boxCollider3DComponent["Offset"].as<glm::vec3>();
					bc3d.HalfExtents = boxCollider3DComponent["HalfExtents"].as<glm::vec3>();
				}
			}
		}

		return true;
	}

	bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
	{
		// Not implemented
		GE_CORE_ASSERT(false);
		return false;
	}

}
