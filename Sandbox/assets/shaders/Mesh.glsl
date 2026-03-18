// 3D Mesh Shader — Blinn-Phong lighting model
// All mutable data arrives via Uniform Buffer Objects so the shader is
// compatible with both the Vulkan SPIR-V and OpenGL SPIR-V compilation paths.

// ============================================================
//  VERTEX STAGE
// ============================================================
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

// Binding 1 — camera data (shared across all 3D draws in a scene)
layout(std140, binding = 1) uniform CameraData
{
	mat4 u_ViewProjection;
	vec4 u_CameraPos; // w unused
};

// Binding 2 — per-object data (updated before every draw call)
layout(std140, binding = 2) uniform ObjectData
{
	mat4 u_Transform;
	vec4 u_Color;
	int  u_EntityID;
	int  _pad0; int _pad1; int _pad2;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_WorldPos;
layout(location = 2) out vec2 v_TexCoord;

void main()
{
	// Normal matrix removes non-uniform scale distortion from normals.
	mat3 normalMatrix = transpose(inverse(mat3(u_Transform)));
	v_Normal   = normalize(normalMatrix * a_Normal);
	v_WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_TexCoord = a_TexCoord;

	gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.0);
}

// ============================================================
//  FRAGMENT STAGE  —  Blinn-Phong
// ============================================================
#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int  o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_WorldPos;
layout(location = 2) in vec2 v_TexCoord;

// Binding 1 — camera (only CameraPos used in the fragment stage)
layout(std140, binding = 1) uniform CameraData
{
	mat4 u_ViewProjection;
	vec4 u_CameraPos;
};

// Binding 2 — per-object
layout(std140, binding = 2) uniform ObjectData
{
	mat4 u_Transform;
	vec4 u_Color;
	int  u_EntityID;
	int  _fpad0; int _fpad1; int _fpad2;
};

// GPU-side directional light representation.
struct DirLightGPU
{
	vec4 Direction; // xyz = world-space direction toward the scene, w = intensity
	vec4 Color;     // xyz = RGB colour, w unused
};

// GPU-side point light representation.
struct PointLightGPU
{
	vec4 Position;    // xyz = world-space position, w = intensity
	vec4 Color;       // xyz = RGB colour, w = constant attenuation
	vec4 Attenuation; // x = linear, y = quadratic, zw unused
};

#define MAX_POINT_LIGHTS 4

// Binding 3 — scene lighting (updated once per BeginScene)
layout(std140, binding = 3) uniform LightData
{
	vec4        u_AmbientColor;             // xyz = colour, w unused
	ivec4       u_LightInfo;                // x = hasDirectional, y = numPointLights
	DirLightGPU u_DirLight;
	PointLightGPU u_PointLights[MAX_POINT_LIGHTS];
};

// ---------------------------------------------------------------------------
// Blinn-Phong directional light contribution.
vec3 CalcDirLight(vec3 norm, vec3 viewDir)
{
	vec3  lightDir = normalize(-u_DirLight.Direction.xyz);
	float intensity = u_DirLight.Direction.w;

	float diff    = max(dot(norm, lightDir), 0.0);
	vec3  halfDir = normalize(lightDir + viewDir);
	float spec    = pow(max(dot(norm, halfDir), 0.0), 32.0);

	vec3 diffuse  = diff * u_DirLight.Color.rgb * intensity;
	vec3 specular = spec * u_DirLight.Color.rgb * intensity * 0.3;
	return diffuse + specular;
}

// ---------------------------------------------------------------------------
// Blinn-Phong point light contribution with quadratic attenuation.
vec3 CalcPointLight(int i, vec3 norm, vec3 viewDir)
{
	vec3  toLight  = u_PointLights[i].Position.xyz - v_WorldPos;
	vec3  lightDir = normalize(toLight);
	float intensity = u_PointLights[i].Position.w;
	float dist      = length(toLight);

	float constant  = u_PointLights[i].Color.w;
	float linear    = u_PointLights[i].Attenuation.x;
	float quadratic = u_PointLights[i].Attenuation.y;
	float atten     = 1.0 / (constant + linear * dist + quadratic * dist * dist);

	float diff    = max(dot(norm, lightDir), 0.0);
	vec3  halfDir = normalize(lightDir + viewDir);
	float spec    = pow(max(dot(norm, halfDir), 0.0), 32.0);

	vec3 diffuse  = diff * u_PointLights[i].Color.rgb * intensity * atten;
	vec3 specular = spec * u_PointLights[i].Color.rgb * intensity * atten * 0.3;
	return diffuse + specular;
}

// ---------------------------------------------------------------------------
void main()
{
	vec3 norm    = normalize(v_Normal);
	vec3 viewDir = normalize(u_CameraPos.xyz - v_WorldPos);

	// Start with ambient term.
	vec3 result = u_AmbientColor.xyz;

	// Directional light.
	if (u_LightInfo.x != 0)
		result += CalcDirLight(norm, viewDir);

	// Point lights.
	int numPoint = min(u_LightInfo.y, MAX_POINT_LIGHTS);
	for (int i = 0; i < numPoint; i++)
		result += CalcPointLight(i, norm, viewDir);

	result *= u_Color.rgb;
	o_Color    = vec4(result, u_Color.a);
	o_EntityID = u_EntityID;
}
