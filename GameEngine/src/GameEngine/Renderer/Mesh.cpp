#include "gepch.h"
#include "GameEngine/Renderer/Mesh.h"

#include "GameEngine/Renderer/Renderer3D.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

namespace GameEngine {

	Mesh::Mesh(const std::vector<MeshVertex>& vertices, const std::vector<uint32_t>& indices)
		: myIndexCount((uint32_t)indices.size())
	{
		myVertexArray = VertexArray::Create();

		auto vb = VertexBuffer::Create(
			(float*)vertices.data(),
			(uint32_t)(vertices.size() * sizeof(MeshVertex))
		);
		vb->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float3, "a_Normal"   },
			{ ShaderDataType::Float2, "a_TexCoord" }
		});
		myVertexArray->AddVertexBuffer(vb);

		auto ib = IndexBuffer::Create(
			(uint32_t*)indices.data(),
			(uint32_t)indices.size()
		);
		myVertexArray->SetIndexBuffer(ib);
	}

	// ---------------------------------------------------------------------------
	// Mesh::Create  —  Assimp-based loader (OBJ, FBX, GLTF, DAE, …)
	// ---------------------------------------------------------------------------

	Handle<Mesh> Mesh::Create(const std::string& filepath)
	{
		Assimp::Importer importer;

		// aiProcess_Triangulate    : convert quads/n-gons to triangles.
		// aiProcess_GenSmoothNormals: compute normals when absent.
		// aiProcess_FlipUVs        : flip V for OpenGL (origin at bottom-left).
		// aiProcess_JoinIdenticalVertices: de-duplicate shared vertices.
		const aiScene* scene = importer.ReadFile(filepath,
			aiProcess_Triangulate          |
			aiProcess_GenSmoothNormals     |
			aiProcess_FlipUVs              |
			aiProcess_JoinIdenticalVertices
		);

		if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || !scene->mRootNode)
		{
			GE_CORE_ERROR("Mesh::Create: failed to load '{0}': {1}",
				filepath, importer.GetErrorString());
			return nullptr;
		}

		std::vector<MeshVertex> vertices;
		std::vector<uint32_t>   indices;

		// Merge all sub-meshes in the file into a single vertex/index buffer.
		for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
		{
			const aiMesh* mesh      = scene->mMeshes[m];
			const uint32_t baseVert = (uint32_t)vertices.size();

			for (uint32_t i = 0; i < mesh->mNumVertices; ++i)
			{
				MeshVertex v;

				v.Position = { mesh->mVertices[i].x,
				               mesh->mVertices[i].y,
				               mesh->mVertices[i].z };

				v.Normal = mesh->HasNormals()
					? glm::vec3{ mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z }
					: glm::vec3{ 0.0f, 1.0f, 0.0f };

				// mTextureCoords[0] is the first UV channel (most models only have one).
				v.TexCoord = mesh->mTextureCoords[0]
					? glm::vec2{ mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y }
					: glm::vec2{ 0.0f, 0.0f };

				vertices.push_back(v);
			}

			for (uint32_t i = 0; i < mesh->mNumFaces; ++i)
			{
				const aiFace& face = mesh->mFaces[i];
				for (uint32_t j = 0; j < face.mNumIndices; ++j)
					indices.push_back(baseVert + face.mIndices[j]);
			}
		}

		if (vertices.empty() || indices.empty())
		{
			GE_CORE_ERROR("Mesh::Create: no geometry found in '{0}'", filepath);
			return nullptr;
		}

		GE_CORE_INFO("Mesh::Create: loaded '{0}' ({1} vertices, {2} triangles)",
			filepath, vertices.size(), indices.size() / 3);

		auto mesh = MakeHandle<Mesh>(vertices, indices);
		mesh->myFilepath = filepath;

		// Extract the diffuse texture from the first material that has one.
		std::string directory = filepath.substr(0, filepath.find_last_of("/\\"));
		for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
		{
			uint32_t matIndex = scene->mMeshes[m]->mMaterialIndex;
			if (matIndex >= scene->mNumMaterials)
				continue;

			const aiMaterial* mat = scene->mMaterials[matIndex];
			if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0)
				continue;

			aiString texPath;
			if (mat->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == AI_SUCCESS)
			{
				std::string fullPath = directory + "/" + texPath.C_Str();
				mesh->myTexture = Renderer3D::LoadTexture(fullPath);
				GE_CORE_INFO("Mesh::Create: loaded texture '{0}'", fullPath);
				break;
			}
		}

		return mesh;
	}

}
