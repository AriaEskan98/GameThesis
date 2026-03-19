#pragma once

#include "GameEngine/Renderer/Buffer.h"
#include "GameEngine/Renderer/Texture.h"
#include "GameEngine/Renderer/VertexArray.h"

#include <glm/glm.hpp>

#include <string>
#include <vector>

namespace GameEngine {

	/// A single vertex in a 3D mesh, holding position, surface normal, and texture coordinates.
	struct MeshVertex
	{
		glm::vec3 Position;
		glm::vec3 Normal;
		glm::vec2 TexCoord;
	};

	/// A 3D mesh loaded via Assimp (OBJ, FBX, GLTF, DAE, …) and stored in GPU-side
	/// vertex/index buffers. Supports positions, normals, and texture coordinates.
	/// Smooth normals are generated automatically when the source file omits them.
	class Mesh
	{
	public:
		/// Construct a mesh directly from pre-built vertex and index data.
		Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices);

		/// Load a mesh from any Assimp-supported format (OBJ, FBX, GLTF, DAE, …).
		/// Returns nullptr on failure.
		static Handle<Mesh> Create(const std::string& filepath);

		const Handle<VertexArray>& GetVertexArray() const { return myVertexArray; }
		uint32_t GetIndexCount() const { return myIndexCount; }
		const std::string& GetFilepath() const { return myFilepath; }
		const Handle<Texture2D>& GetTexture() const { return myTexture; }
	private:
		Handle<VertexArray> myVertexArray;
		uint32_t myIndexCount = 0;
		std::string myFilepath;
		Handle<Texture2D> myTexture;
	};

}
