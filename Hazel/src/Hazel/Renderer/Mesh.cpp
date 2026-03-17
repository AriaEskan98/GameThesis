#include "gepch.h"
#include "Hazel/Renderer/Mesh.h"

#include <fstream>
#include <sstream>
#include <unordered_map>

#include <glm/glm.hpp>

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
	// OBJ parsing helpers
	// ---------------------------------------------------------------------------

	/// Parse an OBJ face-vertex token such as "1", "1/2", "1//3", or "1/2/3".
	/// Indices are converted from 1-based OBJ convention to 0-based.
	/// A value of -1 means the component was absent.
	static void ParseFaceToken(const std::string& token, int& outPos, int& outTex, int& outNorm)
	{
		outPos = outTex = outNorm = -1;

		size_t s1 = token.find('/');
		if (s1 == std::string::npos)
		{
			outPos = std::stoi(token) - 1;
			return;
		}

		outPos = std::stoi(token.substr(0, s1)) - 1;

		size_t s2 = token.find('/', s1 + 1);
		if (s2 == std::string::npos)
		{
			// "v/t"
			if (s1 + 1 < token.size())
				outTex = std::stoi(token.substr(s1 + 1)) - 1;
			return;
		}

		// "v/t/n" or "v//n"
		if (s2 > s1 + 1)
			outTex = std::stoi(token.substr(s1 + 1, s2 - s1 - 1)) - 1;
		if (s2 + 1 < token.size())
			outNorm = std::stoi(token.substr(s2 + 1)) - 1;
	}

	/// Hash for a (posIdx, texIdx, normIdx) triple used to de-duplicate vertices.
	struct IndexTripleHash
	{
		size_t operator()(const std::tuple<int, int, int>& t) const
		{
			size_t h = (size_t)std::get<0>(t);
			h = h * 100003u ^ (size_t)std::get<1>(t);
			h = h * 100003u ^ (size_t)std::get<2>(t);
			return h;
		}
	};

	// ---------------------------------------------------------------------------
	// Mesh::Create
	// ---------------------------------------------------------------------------

	Handle<Mesh> Mesh::Create(const std::string& filepath)
	{
		std::ifstream file(filepath);
		if (!file.is_open())
		{
			GE_CORE_ERROR("Mesh::Create: could not open '{0}'", filepath);
			return nullptr;
		}

		// Raw OBJ data
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> normals;
		std::vector<glm::vec2> texcoords;

		// Output geometry
		std::unordered_map<std::tuple<int, int, int>, uint32_t, IndexTripleHash> vertexCache;
		std::vector<MeshVertex> vertices;
		std::vector<uint32_t>   indices;

		std::string line;
		while (std::getline(file, line))
		{
			if (line.empty() || line[0] == '#')
				continue;

			std::istringstream ss(line);
			std::string token;
			ss >> token;

			if (token == "v")
			{
				glm::vec3 p;
				ss >> p.x >> p.y >> p.z;
				positions.push_back(p);
			}
			else if (token == "vn")
			{
				glm::vec3 n;
				ss >> n.x >> n.y >> n.z;
				normals.push_back(n);
			}
			else if (token == "vt")
			{
				glm::vec2 t;
				ss >> t.x >> t.y;
				texcoords.push_back(t);
			}
			else if (token == "f")
			{
				// Collect all vertex indices for this face, then triangulate as a fan.
				std::vector<uint32_t> faceVerts;
				std::string vtok;
				while (ss >> vtok)
				{
					int pi, ti, ni;
					ParseFaceToken(vtok, pi, ti, ni);

					auto key = std::make_tuple(pi, ti, ni);
					auto it  = vertexCache.find(key);
					if (it != vertexCache.end())
					{
						faceVerts.push_back(it->second);
					}
					else
					{
						MeshVertex v;
						v.Position = (pi >= 0 && pi < (int)positions.size()) ? positions[pi] : glm::vec3(0.0f);
						v.Normal   = (ni >= 0 && ni < (int)normals.size())   ? normals[ni]   : glm::vec3(0.0f, 1.0f, 0.0f);
						v.TexCoord = (ti >= 0 && ti < (int)texcoords.size()) ? texcoords[ti] : glm::vec2(0.0f);

						uint32_t idx = (uint32_t)vertices.size();
						vertices.push_back(v);
						vertexCache[key] = idx;
						faceVerts.push_back(idx);
					}
				}

				// Fan triangulation: (0,1,2), (0,2,3), (0,3,4), ...
				for (size_t i = 1; i + 1 < faceVerts.size(); ++i)
				{
					indices.push_back(faceVerts[0]);
					indices.push_back(faceVerts[i]);
					indices.push_back(faceVerts[i + 1]);
				}
			}
		}

		if (vertices.empty() || indices.empty())
		{
			GE_CORE_ERROR("Mesh::Create: no geometry found in '{0}'", filepath);
			return nullptr;
		}

		// If the OBJ did not supply normals, compute smooth normals by accumulating
		// face normals at shared vertices and normalizing.
		if (normals.empty())
		{
			for (auto& v : vertices)
				v.Normal = glm::vec3(0.0f);

			for (size_t i = 0; i + 2 < indices.size(); i += 3)
			{
				MeshVertex& v0 = vertices[indices[i]];
				MeshVertex& v1 = vertices[indices[i + 1]];
				MeshVertex& v2 = vertices[indices[i + 2]];
				glm::vec3 n = glm::normalize(
					glm::cross(v1.Position - v0.Position, v2.Position - v0.Position));
				v0.Normal += n;
				v1.Normal += n;
				v2.Normal += n;
			}
			for (auto& v : vertices)
				v.Normal = glm::normalize(v.Normal);
		}

		GE_CORE_INFO("Mesh::Create: loaded '{0}' ({1} vertices, {2} triangles)",
			filepath, vertices.size(), indices.size() / 3);

		auto mesh = MakeHandle<Mesh>(vertices, indices);
		mesh->myFilepath = filepath;
		return mesh;
	}

}
