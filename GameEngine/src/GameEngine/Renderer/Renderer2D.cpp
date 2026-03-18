#include "gepch.h"
#include "GameEngine/Renderer/Renderer2D.h"

#include "GameEngine/Renderer/VertexArray.h"
#include "GameEngine/Renderer/Shader.h"
#include "GameEngine/Renderer/UniformBuffer.h"
#include "GameEngine/Renderer/RenderCommand.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MSDFData.h"

namespace GameEngine {

	struct QuadVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;
		float TexIndex;
		float TilingFactor;
		
		// Editor-only
		int EntityID;
	};

	struct CircleVertex
	{
		glm::vec3 WorldPosition;
		glm::vec3 LocalPosition;
		glm::vec4 Color;
		float Thickness;
		float Fade;

		// Editor-only
		int EntityID;
	};

	struct LineVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;

		// Editor-only
		int EntityID;
	};

	struct TextVertex
	{
		glm::vec3 Position;
		glm::vec4 Color;
		glm::vec2 TexCoord;

		// TODO: bg color for outline/bg

		// Editor-only
		int EntityID;
	};

	struct Renderer2DData
	{
		static const uint32_t MaxQuads = 20000;
		static const uint32_t MaxVertices = MaxQuads * 4;
		static const uint32_t MaxIndices = MaxQuads * 6;
		static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

		Handle<VertexArray> QuadVertexArray;
		Handle<VertexBuffer> QuadVertexBuffer;
		Handle<Shader> QuadShader;
		Handle<Texture2D> WhiteTexture;

		Handle<VertexArray> CircleVertexArray;
		Handle<VertexBuffer> CircleVertexBuffer;
		Handle<Shader> CircleShader;

		Handle<VertexArray> LineVertexArray;
		Handle<VertexBuffer> LineVertexBuffer;
		Handle<Shader> LineShader;	
		
		Handle<VertexArray> TextVertexArray;
		Handle<VertexBuffer> TextVertexBuffer;
		Handle<Shader> TextShader;

		uint32_t QuadIndexCount = 0;
		QuadVertex* QuadVertexBufferBase = nullptr;
		QuadVertex* QuadVertexBufferPtr = nullptr;

		uint32_t CircleIndexCount = 0;
		CircleVertex* CircleVertexBufferBase = nullptr;
		CircleVertex* CircleVertexBufferPtr = nullptr;

		uint32_t LineVertexCount = 0;
		LineVertex* LineVertexBufferBase = nullptr;
		LineVertex* LineVertexBufferPtr = nullptr;

		uint32_t TextIndexCount = 0;
		TextVertex* TextVertexBufferBase = nullptr;
		TextVertex* TextVertexBufferPtr = nullptr;

		float LineWidth = 2.0f;

		std::array<Handle<Texture2D>, MaxTextureSlots> TextureSlots;
		uint32_t TextureSlotIndex = 1; // 0 = white texture
		std::unordered_map<Texture2D*, float> TextureSlotMap;

		Handle<Texture2D> FontAtlasTexture;

		glm::vec4 QuadVertexPositions[4];

		Renderer2D::Statistics Stats;

		struct CameraData
		{
			glm::mat4 ViewProjection;
		};
		CameraData CameraBuffer;
		Handle<UniformBuffer> CameraUniformBuffer;
	};

	static Renderer2DData gsData;

	void Renderer2D::Init()
	{
		GE_PROFILE_FUNCTION();

		gsData.QuadVertexArray = VertexArray::Create();

		gsData.QuadVertexBuffer = VertexBuffer::Create(gsData.MaxVertices * sizeof(QuadVertex));
		gsData.QuadVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoord"     },
			{ ShaderDataType::Float,  "a_TexIndex"     },
			{ ShaderDataType::Float,  "a_TilingFactor" },
			{ ShaderDataType::Int,    "a_EntityID"     }
		});
		gsData.QuadVertexArray->AddVertexBuffer(gsData.QuadVertexBuffer);

		gsData.QuadVertexBufferBase = new QuadVertex[gsData.MaxVertices];

		uint32_t* quadIndices = new uint32_t[gsData.MaxIndices];

		uint32_t offset = 0;
		for (uint32_t i = 0; i < gsData.MaxIndices; i += 6)
		{
			quadIndices[i + 0] = offset + 0;
			quadIndices[i + 1] = offset + 1;
			quadIndices[i + 2] = offset + 2;

			quadIndices[i + 3] = offset + 2;
			quadIndices[i + 4] = offset + 3;
			quadIndices[i + 5] = offset + 0;

			offset += 4;
		}

		Handle<IndexBuffer> quadIB = IndexBuffer::Create(quadIndices, gsData.MaxIndices);
		gsData.QuadVertexArray->SetIndexBuffer(quadIB);
		delete[] quadIndices;

		// Circles
		gsData.CircleVertexArray = VertexArray::Create();

		gsData.CircleVertexBuffer = VertexBuffer::Create(gsData.MaxVertices * sizeof(CircleVertex));
		gsData.CircleVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_WorldPosition" },
			{ ShaderDataType::Float3, "a_LocalPosition" },
			{ ShaderDataType::Float4, "a_Color"         },
			{ ShaderDataType::Float,  "a_Thickness"     },
			{ ShaderDataType::Float,  "a_Fade"          },
			{ ShaderDataType::Int,    "a_EntityID"      }
		});
		gsData.CircleVertexArray->AddVertexBuffer(gsData.CircleVertexBuffer);
		gsData.CircleVertexArray->SetIndexBuffer(quadIB); // Use quad IB
		gsData.CircleVertexBufferBase = new CircleVertex[gsData.MaxVertices];

		// Lines
		gsData.LineVertexArray = VertexArray::Create();

		gsData.LineVertexBuffer = VertexBuffer::Create(gsData.MaxVertices * sizeof(LineVertex));
		gsData.LineVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position" },
			{ ShaderDataType::Float4, "a_Color"    },
			{ ShaderDataType::Int,    "a_EntityID" }
		});
		gsData.LineVertexArray->AddVertexBuffer(gsData.LineVertexBuffer);
		gsData.LineVertexBufferBase = new LineVertex[gsData.MaxVertices];

		// Text
		gsData.TextVertexArray = VertexArray::Create();

		gsData.TextVertexBuffer = VertexBuffer::Create(gsData.MaxVertices * sizeof(TextVertex));
		gsData.TextVertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_Position"     },
			{ ShaderDataType::Float4, "a_Color"        },
			{ ShaderDataType::Float2, "a_TexCoord"     },
			{ ShaderDataType::Int,    "a_EntityID"     }
		});
		gsData.TextVertexArray->AddVertexBuffer(gsData.TextVertexBuffer);
		gsData.TextVertexArray->SetIndexBuffer(quadIB);
		gsData.TextVertexBufferBase = new TextVertex[gsData.MaxVertices];

		gsData.WhiteTexture = Texture2D::Create(TextureSpecification());
		uint32_t whiteTextureData = 0xffffffff;
		gsData.WhiteTexture->SetData(&whiteTextureData, sizeof(uint32_t));

		int32_t samplers[gsData.MaxTextureSlots];
		for (uint32_t i = 0; i < gsData.MaxTextureSlots; i++)
			samplers[i] = i;

		gsData.QuadShader = Shader::Create("assets/shaders/Renderer2D_Quad.glsl");
		gsData.CircleShader = Shader::Create("assets/shaders/Renderer2D_Circle.glsl");
		gsData.LineShader = Shader::Create("assets/shaders/Renderer2D_Line.glsl");
		gsData.TextShader = Shader::Create("assets/shaders/Renderer2D_Text.glsl");

		// Set first texture slot to 0
		gsData.TextureSlots[0] = gsData.WhiteTexture;

		gsData.QuadVertexPositions[0] = { -0.5f, -0.5f, 0.0f, 1.0f };
		gsData.QuadVertexPositions[1] = {  0.5f, -0.5f, 0.0f, 1.0f };
		gsData.QuadVertexPositions[2] = {  0.5f,  0.5f, 0.0f, 1.0f };
		gsData.QuadVertexPositions[3] = { -0.5f,  0.5f, 0.0f, 1.0f };

		gsData.CameraUniformBuffer = UniformBuffer::Create(sizeof(Renderer2DData::CameraData), 0);
	}

	void Renderer2D::Shutdown()
	{
		GE_PROFILE_FUNCTION();

		delete[] gsData.QuadVertexBufferBase;
	}

	void Renderer2D::BeginScene(const OrthographicCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		gsData.CameraBuffer.ViewProjection = camera.GetViewProjectionMatrix();
		gsData.CameraUniformBuffer->SetData(&gsData.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform)
	{
		GE_PROFILE_FUNCTION();

		gsData.CameraBuffer.ViewProjection = camera.GetProjection() * glm::inverse(transform);
		gsData.CameraUniformBuffer->SetData(&gsData.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::BeginScene(const EditorCamera& camera)
	{
		GE_PROFILE_FUNCTION();

		gsData.CameraBuffer.ViewProjection = camera.GetViewProjection();
		gsData.CameraUniformBuffer->SetData(&gsData.CameraBuffer, sizeof(Renderer2DData::CameraData));

		StartBatch();
	}

	void Renderer2D::EndScene()
	{
		GE_PROFILE_FUNCTION();

		Flush();
	}

	void Renderer2D::StartBatch()
	{
		gsData.QuadIndexCount = 0;
		gsData.QuadVertexBufferPtr = gsData.QuadVertexBufferBase;

		gsData.CircleIndexCount = 0;
		gsData.CircleVertexBufferPtr = gsData.CircleVertexBufferBase;

		gsData.LineVertexCount = 0;
		gsData.LineVertexBufferPtr = gsData.LineVertexBufferBase;	
		
		gsData.TextIndexCount = 0;
		gsData.TextVertexBufferPtr = gsData.TextVertexBufferBase;

		gsData.TextureSlotIndex = 1;
		gsData.TextureSlotMap.clear();
	}

	void Renderer2D::Flush()
	{
		if (gsData.QuadIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)gsData.QuadVertexBufferPtr - (uint8_t*)gsData.QuadVertexBufferBase);
			gsData.QuadVertexBuffer->SetData(gsData.QuadVertexBufferBase, dataSize);

			// Bind textures
			for (uint32_t i = 0; i < gsData.TextureSlotIndex; i++)
				gsData.TextureSlots[i]->Bind(i);

			gsData.QuadShader->Bind();
			RenderCommand::DrawIndexed(gsData.QuadVertexArray, gsData.QuadIndexCount);
			gsData.Stats.DrawCalls++;
		}

		if (gsData.CircleIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)gsData.CircleVertexBufferPtr - (uint8_t*)gsData.CircleVertexBufferBase);
			gsData.CircleVertexBuffer->SetData(gsData.CircleVertexBufferBase, dataSize);

			gsData.CircleShader->Bind();
			RenderCommand::DrawIndexed(gsData.CircleVertexArray, gsData.CircleIndexCount);
			gsData.Stats.DrawCalls++;
		}

		if (gsData.LineVertexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)gsData.LineVertexBufferPtr - (uint8_t*)gsData.LineVertexBufferBase);
			gsData.LineVertexBuffer->SetData(gsData.LineVertexBufferBase, dataSize);

			gsData.LineShader->Bind();
			RenderCommand::SetLineWidth(gsData.LineWidth);
			RenderCommand::DrawLines(gsData.LineVertexArray, gsData.LineVertexCount);
			gsData.Stats.DrawCalls++;
		}
		
		if (gsData.TextIndexCount)
		{
			uint32_t dataSize = (uint32_t)((uint8_t*)gsData.TextVertexBufferPtr - (uint8_t*)gsData.TextVertexBufferBase);
			gsData.TextVertexBuffer->SetData(gsData.TextVertexBufferBase, dataSize);

			auto buf = gsData.TextVertexBufferBase;
			gsData.FontAtlasTexture->Bind(0);

			gsData.TextShader->Bind();
			RenderCommand::DrawIndexed(gsData.TextVertexArray, gsData.TextIndexCount);
			gsData.Stats.DrawCalls++;
		}
	}

	void Renderer2D::NextBatch()
	{
		Flush();
		StartBatch();
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, color);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });
		
		DrawQuad(transform, color);
	}

	void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const Handle<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawQuad({ position.x, position.y, 0.0f }, size, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const Handle<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		GE_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;
		const float textureIndex = 0.0f; // White Texture
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };
		const float tilingFactor = 1.0f;

		if (gsData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			gsData.QuadVertexBufferPtr->Position = transform * gsData.QuadVertexPositions[i];
			gsData.QuadVertexBufferPtr->Color = color;
			gsData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			gsData.QuadVertexBufferPtr->TexIndex = textureIndex;
			gsData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			gsData.QuadVertexBufferPtr->EntityID = entityID;
			gsData.QuadVertexBufferPtr++;
		}

		gsData.QuadIndexCount += 6;

		gsData.Stats.QuadCount++;
	}

	void Renderer2D::DrawQuad(const glm::mat4& transform, const Handle<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor, int entityID)
	{
		GE_PROFILE_FUNCTION();

		constexpr size_t quadVertexCount = 4;
		constexpr glm::vec2 textureCoords[] = { { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f } };

		if (gsData.QuadIndexCount >= Renderer2DData::MaxIndices)
			NextBatch();

		float textureIndex = 0.0f;
		auto slotIt = gsData.TextureSlotMap.find(texture.get());
		if (slotIt != gsData.TextureSlotMap.end())
		{
			textureIndex = slotIt->second;
		}

		if (textureIndex == 0.0f)
		{
			if (gsData.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
				NextBatch();

			textureIndex = (float)gsData.TextureSlotIndex;
			gsData.TextureSlotMap[texture.get()] = textureIndex;
			gsData.TextureSlots[gsData.TextureSlotIndex] = texture;
			gsData.TextureSlotIndex++;
		}

		for (size_t i = 0; i < quadVertexCount; i++)
		{
			gsData.QuadVertexBufferPtr->Position = transform * gsData.QuadVertexPositions[i];
			gsData.QuadVertexBufferPtr->Color = tintColor;
			gsData.QuadVertexBufferPtr->TexCoord = textureCoords[i];
			gsData.QuadVertexBufferPtr->TexIndex = textureIndex;
			gsData.QuadVertexBufferPtr->TilingFactor = tilingFactor;
			gsData.QuadVertexBufferPtr->EntityID = entityID;
			gsData.QuadVertexBufferPtr++;
		}

		gsData.QuadIndexCount += 6;

		gsData.Stats.QuadCount++;
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, color);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Handle<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		DrawRotatedQuad({ position.x, position.y, 0.0f }, size, rotation, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Handle<Texture2D>& texture, float tilingFactor, const glm::vec4& tintColor)
	{
		GE_PROFILE_FUNCTION();

		glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
			* glm::rotate(glm::mat4(1.0f), glm::radians(rotation), { 0.0f, 0.0f, 1.0f })
			* glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

		DrawQuad(transform, texture, tilingFactor, tintColor);
	}

	void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness /*= 1.0f*/, float fade /*= 0.005f*/, int entityID /*= -1*/)
	{
		GE_PROFILE_FUNCTION();

		// TODO: implement for circles
		// if (gsData.QuadIndexCount >= Renderer2DData::MaxIndices)
		// 	NextBatch();

		for (size_t i = 0; i < 4; i++)
		{
			gsData.CircleVertexBufferPtr->WorldPosition = transform * gsData.QuadVertexPositions[i];
			gsData.CircleVertexBufferPtr->LocalPosition = gsData.QuadVertexPositions[i] * 2.0f;
			gsData.CircleVertexBufferPtr->Color = color;
			gsData.CircleVertexBufferPtr->Thickness = thickness;
			gsData.CircleVertexBufferPtr->Fade = fade;
			gsData.CircleVertexBufferPtr->EntityID = entityID;
			gsData.CircleVertexBufferPtr++;
		}

		gsData.CircleIndexCount += 6;

		gsData.Stats.QuadCount++;
	}

	void Renderer2D::DrawLine(const glm::vec3& p0, glm::vec3& p1, const glm::vec4& color, int entityID)
	{
		gsData.LineVertexBufferPtr->Position = p0;
		gsData.LineVertexBufferPtr->Color = color;
		gsData.LineVertexBufferPtr->EntityID = entityID;
		gsData.LineVertexBufferPtr++;

		gsData.LineVertexBufferPtr->Position = p1;
		gsData.LineVertexBufferPtr->Color = color;
		gsData.LineVertexBufferPtr->EntityID = entityID;
		gsData.LineVertexBufferPtr++;

		gsData.LineVertexCount += 2;
	}

	void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID)
	{
		glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
		glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
		glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

		DrawLine(p0, p1, color, entityID);
		DrawLine(p1, p2, color, entityID);
		DrawLine(p2, p3, color, entityID);
		DrawLine(p3, p0, color, entityID);
	}

	void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID)
	{
		glm::vec3 lineVertices[4];
		for (size_t i = 0; i < 4; i++)
			lineVertices[i] = transform * gsData.QuadVertexPositions[i];

		DrawLine(lineVertices[0], lineVertices[1], color, entityID);
		DrawLine(lineVertices[1], lineVertices[2], color, entityID);
		DrawLine(lineVertices[2], lineVertices[3], color, entityID);
		DrawLine(lineVertices[3], lineVertices[0], color, entityID);
	}

	void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID)
	{
		if (src.Texture)
			DrawQuad(transform, src.Texture, src.TilingFactor, src.Color, entityID);
		else
			DrawQuad(transform, src.Color, entityID);
	}

	void Renderer2D::DrawString(const std::string& string, Handle<Font> font, const glm::mat4& transform, const TextParams& textParams, int entityID)
	{
		const auto& fontGeometry = font->GetMSDFData()->FontGeometry;
		const auto& metrics = fontGeometry.getMetrics();
		Handle<Texture2D> fontAtlas = font->GetAtlasTexture();

		gsData.FontAtlasTexture = fontAtlas;

		double x = 0.0;
		double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
		double y = 0.0;

		const float spaceGlyphAdvance = fontGeometry.getGlyph(' ')->getAdvance();
		
		for (size_t i = 0; i < string.size(); i++)
		{
			char character = string[i];
			if (character == '\r')
				continue;

			if (character == '\n')
			{
				x = 0;
				y -= fsScale * metrics.lineHeight + textParams.LineSpacing;
				continue;
			}

			if (character == ' ')
			{
				float advance = spaceGlyphAdvance;
				if (i < string.size() - 1)
				{
					char nextCharacter = string[i + 1];
					double dAdvance;
					fontGeometry.getAdvance(dAdvance, character, nextCharacter);
					advance = (float)dAdvance;
				}

				x += fsScale * advance + textParams.Kerning;
				continue;
			}

			if (character == '\t')
			{
				// NOTE(Yan): is this right?
				x += 4.0f * (fsScale * spaceGlyphAdvance + textParams.Kerning);
				continue;
			}

			auto glyph = fontGeometry.getGlyph(character);
			if (!glyph)
				glyph = fontGeometry.getGlyph('?');
			if (!glyph)
				return;

			double al, ab, ar, at;
			glyph->getQuadAtlasBounds(al, ab, ar, at);
			glm::vec2 texCoordMin((float)al, (float)ab);
			glm::vec2 texCoordMax((float)ar, (float)at);

			double pl, pb, pr, pt;
			glyph->getQuadPlaneBounds(pl, pb, pr, pt);
			glm::vec2 quadMin((float)pl, (float)pb);
			glm::vec2 quadMax((float)pr, (float)pt);

			quadMin *= fsScale, quadMax *= fsScale;
			quadMin += glm::vec2(x, y);
			quadMax += glm::vec2(x, y);

			float texelWidth = 1.0f / fontAtlas->GetWidth();
			float texelHeight = 1.0f / fontAtlas->GetHeight();
			texCoordMin *= glm::vec2(texelWidth, texelHeight);
			texCoordMax *= glm::vec2(texelWidth, texelHeight);

			// render here
			gsData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
			gsData.TextVertexBufferPtr->Color = textParams.Color;
			gsData.TextVertexBufferPtr->TexCoord = texCoordMin;
			gsData.TextVertexBufferPtr->EntityID = entityID;
			gsData.TextVertexBufferPtr++;

			gsData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
			gsData.TextVertexBufferPtr->Color = textParams.Color;
			gsData.TextVertexBufferPtr->TexCoord = { texCoordMin.x, texCoordMax.y };
			gsData.TextVertexBufferPtr->EntityID = entityID;
			gsData.TextVertexBufferPtr++;

			gsData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
			gsData.TextVertexBufferPtr->Color = textParams.Color;
			gsData.TextVertexBufferPtr->TexCoord = texCoordMax;
			gsData.TextVertexBufferPtr->EntityID = entityID;
			gsData.TextVertexBufferPtr++;

			gsData.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
			gsData.TextVertexBufferPtr->Color = textParams.Color;
			gsData.TextVertexBufferPtr->TexCoord = { texCoordMax.x, texCoordMin.y };
			gsData.TextVertexBufferPtr->EntityID = entityID;
			gsData.TextVertexBufferPtr++;

			gsData.TextIndexCount += 6;
			gsData.Stats.QuadCount++;

			if (i < string.size() - 1)
			{
				double advance = glyph->getAdvance();
				char nextCharacter = string[i + 1];
				fontGeometry.getAdvance(advance, character, nextCharacter);

				x += fsScale * advance + textParams.Kerning;
			}
		}
	}

	void Renderer2D::DrawString(const std::string& string, const glm::mat4& transform, const TextComponent& component, int entityID)
	{
		DrawString(string, component.FontAsset, transform, { component.Color, component.Kerning, component.LineSpacing }, entityID);
	}

	float Renderer2D::GetLineWidth()
	{
		return gsData.LineWidth;
	}

	void Renderer2D::SetLineWidth(float width)
	{
		gsData.LineWidth = width;
	}

	void Renderer2D::ResetStats()
	{
		memset(&gsData.Stats, 0, sizeof(Statistics));
	}

	Renderer2D::Statistics Renderer2D::GetStats()
	{
		return gsData.Stats;
	}

}
