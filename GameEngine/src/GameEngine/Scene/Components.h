#pragma once

#include "SceneCamera.h"
#include "GameEngine/Core/UUID.h"
#include "GameEngine/Renderer/Texture.h"
#include "GameEngine/Renderer/Font.h"
#include "GameEngine/Renderer/Mesh.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace GameEngine {

	struct IDComponent
	{
		UUID ID;

		IDComponent() = default;
		IDComponent(const IDComponent&) = default;
	};

	struct TagComponent
	{
		std::string Tag;

		TagComponent() = default;
		TagComponent(const TagComponent&) = default;
		TagComponent(const std::string& tag)
			: Tag(tag) {}
	};

	struct TransformComponent
	{
		glm::vec3 Translation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Rotation = { 0.0f, 0.0f, 0.0f };
		glm::vec3 Scale = { 1.0f, 1.0f, 1.0f };

		TransformComponent() = default;
		TransformComponent(const TransformComponent&) = default;
		TransformComponent(const glm::vec3& translation)
			: Translation(translation) {}

		glm::mat4 GetTransform() const
		{
			glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

			return glm::translate(glm::mat4(1.0f), Translation)
				* rotation
				* glm::scale(glm::mat4(1.0f), Scale);
		}
	};

	struct SpriteRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		Handle<Texture2D> Texture;
		float TilingFactor = 1.0f;

		SpriteRendererComponent() = default;
		SpriteRendererComponent(const SpriteRendererComponent&) = default;
		SpriteRendererComponent(const glm::vec4& color)
			: Color(color) {}
	};

	struct CircleRendererComponent
	{
		glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
		float Thickness = 1.0f;
		float Fade = 0.005f;

		CircleRendererComponent() = default;
		CircleRendererComponent(const CircleRendererComponent&) = default;
	};

	struct CameraComponent
	{
		SceneCamera Camera;
		bool Primary = true; // TODO: think about moving to Scene
		bool FixedAspectRatio = false;

		CameraComponent() = default;
		CameraComponent(const CameraComponent&) = default;
	};

	struct ScriptComponent
	{
		std::string ClassName;

		ScriptComponent() = default;
		ScriptComponent(const ScriptComponent&) = default;
	};

	// Forward declaration
	class ScriptableEntity;

	struct NativeScriptComponent
	{
		ScriptableEntity* Instance = nullptr;

		ScriptableEntity*(*InstantiateScript)();
		void (*DestroyScript)(NativeScriptComponent*);

		template<typename T>
		void Bind()
		{
			InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
			DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
		}
	};

	// Physics

	struct Rigidbody2DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };
		BodyType Type = BodyType::Static;
		bool FixedRotation = false;

		// Storage for runtime
		void* RuntimeBody = nullptr;

		Rigidbody2DComponent() = default;
		Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
	};

	struct BoxCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		glm::vec2 Size = { 0.5f, 0.5f };

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		BoxCollider2DComponent() = default;
		BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
	};

	struct CircleCollider2DComponent
	{
		glm::vec2 Offset = { 0.0f, 0.0f };
		float Radius = 0.5f;

		// TODO(Yan): move into physics material in the future maybe
		float Density = 1.0f;
		float Friction = 0.5f;
		float Restitution = 0.0f;
		float RestitutionThreshold = 0.5f;

		// Storage for runtime
		void* RuntimeFixture = nullptr;

		CircleCollider2DComponent() = default;
		CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
	};

	struct TextComponent
	{
		std::string TextString;
		Handle<Font> FontAsset = Font::GetDefault();
		glm::vec4 Color{ 1.0f };
		float Kerning = 0.0f;
		float LineSpacing = 0.0f;
	};

	/// Casts parallel light rays from a fixed direction (derived from the entity's rotation).
	/// Only the first DirectionalLightComponent in the scene is used.
	struct DirectionalLightComponent
	{
		glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
		float     Intensity = 1.0f;

		DirectionalLightComponent() = default;
		DirectionalLightComponent(const DirectionalLightComponent&) = default;
	};

	/// A positional light source that attenuates with distance.
	/// Up to Renderer3D::MaxPointLights (4) are active simultaneously.
	struct PointLightComponent
	{
		glm::vec3 Color     = { 1.0f, 1.0f, 1.0f };
		float     Intensity = 1.0f;
		float     Constant  = 1.0f;   ///< Constant attenuation term.
		float     Linear    = 0.09f;  ///< Linear attenuation term.
		float     Quadratic = 0.032f; ///< Quadratic attenuation term.

		PointLightComponent() = default;
		PointLightComponent(const PointLightComponent&) = default;
	};

	/// Attaches a 3D mesh to an entity so that it is drawn by Renderer3D each frame.
	struct MeshRendererComponent
	{
		Handle<Mesh> Mesh;
		glm::vec4 Color = { 1.0f, 1.0f, 1.0f, 1.0f };

		MeshRendererComponent() = default;
		MeshRendererComponent(const MeshRendererComponent&) = default;
	};

	// -----------------------------------------------------------------------
	// 3-D physics components
	// -----------------------------------------------------------------------

	/// Rigid body for the 3-D physics simulation.
	/// A runtime pointer to the Physics3DBody is stored in RuntimeBody.
	struct Rigidbody3DComponent
	{
		enum class BodyType { Static = 0, Dynamic, Kinematic };

		BodyType Type     = BodyType::Dynamic;
		float    Mass     = 1.0f;      ///< Ignored when Type == Static.
		float    Friction    = 0.5f;
		float    Restitution = 0.0f;
		bool     UseGravity  = true;

		// Set by the physics system at runtime — do not touch in the editor.
		void* RuntimeBody = nullptr;

		Rigidbody3DComponent() = default;
		Rigidbody3DComponent(const Rigidbody3DComponent&) = default;
	};

	/// An axis-aligned box collider for 3-D physics.
	struct BoxCollider3DComponent
	{
		glm::vec3 Offset      = { 0.0f, 0.0f, 0.0f };
		glm::vec3 HalfExtents = { 0.5f, 0.5f, 0.5f };

		BoxCollider3DComponent() = default;
		BoxCollider3DComponent(const BoxCollider3DComponent&) = default;
	};

	template<typename... Component>
	struct ComponentGroup
	{
	};

	using AllComponents =
		ComponentGroup<TransformComponent, SpriteRendererComponent,
			CircleRendererComponent, CameraComponent, ScriptComponent,
			NativeScriptComponent, Rigidbody2DComponent, BoxCollider2DComponent,
			CircleCollider2DComponent, TextComponent, MeshRendererComponent,
			DirectionalLightComponent, PointLightComponent,
			Rigidbody3DComponent, BoxCollider3DComponent>;

}
