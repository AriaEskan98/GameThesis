// 3D Mesh Shader — Blinn-Phong + normal map + RMA support

// ============================================================
//  VERTEX STAGE
// ============================================================
#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec3 a_Tangent;
layout(location = 3) in vec2 a_TexCoord;

layout(std140, binding = 1) uniform CameraData
{
	mat4 u_ViewProjection;
	vec4 u_CameraPos;
};

layout(std140, binding = 2) uniform ObjectData
{
	mat4 u_Transform;
	vec4 u_Color;
	int  u_EntityID;
	int  u_HasNormalMap;
	int  u_HasRMAMap;
	int  _pad;
};

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_WorldPos;
layout(location = 2) out vec2 v_TexCoord;
layout(location = 3) out mat3 v_TBN;  // occupies locations 3, 4, 5

void main()
{
	mat3 normalMatrix = transpose(inverse(mat3(u_Transform)));

	vec3 N = normalize(normalMatrix * a_Normal);
	vec3 T = normalize(normalMatrix * a_Tangent);
	T = normalize(T - dot(T, N) * N); // re-orthogonalize against N
	vec3 B = cross(N, T);

	v_Normal   = N;
	v_WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_TexCoord = a_TexCoord;
	v_TBN      = mat3(T, B, N);

	gl_Position = u_ViewProjection * vec4(v_WorldPos, 1.0);
}

// ============================================================
//  FRAGMENT STAGE
// ============================================================
#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int  o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_WorldPos;
layout(location = 2) in vec2 v_TexCoord;
layout(location = 3) in mat3 v_TBN;

layout(std140, binding = 1) uniform CameraData
{
	mat4 u_ViewProjection;
	vec4 u_CameraPos;
};

layout(std140, binding = 2) uniform ObjectData
{
	mat4 u_Transform;
	vec4 u_Color;
	int  u_EntityID;
	int  u_HasNormalMap;
	int  u_HasRMAMap;
	int  _fpad;
};

struct DirLightGPU
{
	vec4 Direction; // xyz = world-space direction, w = intensity
	vec4 Color;     // xyz = RGB colour
};

struct PointLightGPU
{
	vec4 Position;    // xyz = world-space position, w = intensity
	vec4 Color;       // xyz = RGB colour, w = constant attenuation
	vec4 Attenuation; // x = linear, y = quadratic
};

#define MAX_POINT_LIGHTS 4

layout(binding = 0) uniform sampler2D u_Texture;
layout(binding = 1) uniform sampler2D u_NormalMap;
layout(binding = 2) uniform sampler2D u_RMAMap;

layout(std140, binding = 3) uniform LightData
{
	vec4          u_AmbientColor;
	ivec4         u_LightInfo;   // x = hasDirectional, y = numPointLights
	DirLightGPU   u_DirLight;
	PointLightGPU u_PointLights[MAX_POINT_LIGHTS];
};

// ---------------------------------------------------------------------------
vec3 CalcDirLight(vec3 norm, vec3 viewDir, float shininess)
{
	vec3  lightDir  = normalize(-u_DirLight.Direction.xyz);
	float intensity = u_DirLight.Direction.w;

	float diff    = max(dot(norm, lightDir), 0.0);
	vec3  halfDir = normalize(lightDir + viewDir);
	float spec    = pow(max(dot(norm, halfDir), 0.0), shininess);

	vec3 diffuse  = diff * u_DirLight.Color.rgb * intensity;
	vec3 specular = spec * u_DirLight.Color.rgb * intensity * 0.3;
	return diffuse + specular;
}

// ---------------------------------------------------------------------------
vec3 CalcPointLight(int i, vec3 norm, vec3 viewDir, float shininess)
{
	vec3  toLight   = u_PointLights[i].Position.xyz - v_WorldPos;
	vec3  lightDir  = normalize(toLight);
	float intensity = u_PointLights[i].Position.w;
	float dist      = length(toLight);

	float constant  = u_PointLights[i].Color.w;
	float linear    = u_PointLights[i].Attenuation.x;
	float quadratic = u_PointLights[i].Attenuation.y;
	float atten     = 1.0 / (constant + linear * dist + quadratic * dist * dist);

	float diff    = max(dot(norm, lightDir), 0.0);
	vec3  halfDir = normalize(lightDir + viewDir);
	float spec    = pow(max(dot(norm, halfDir), 0.0), shininess);

	vec3 diffuse  = diff * u_PointLights[i].Color.rgb * intensity * atten;
	vec3 specular = spec * u_PointLights[i].Color.rgb * intensity * atten * 0.3;
	return diffuse + specular;
}

// ---------------------------------------------------------------------------
void main()
{
	// Normal — from map (tangent space → world) or interpolated vertex normal.
	vec3 norm;
	if (u_HasNormalMap != 0)
	{
		vec3 n = texture(u_NormalMap, v_TexCoord).rgb * 2.0 - 1.0;
		norm = normalize(v_TBN * n);
	}
	else
	{
		norm = normalize(v_Normal);
	}

	// RMA map: R = roughness, G = metalness, B = ambient occlusion.
	float roughness = 0.5;
	float ao        = 1.0;
	if (u_HasRMAMap != 0)
	{
		vec3 rma = texture(u_RMAMap, v_TexCoord).rgb;
		roughness = rma.r;
		ao        = rma.b;
	}

	// Roughness drives specular shininess: smooth surfaces get tight highlights.
	float shininess = mix(8.0, 128.0, 1.0 - roughness);

	vec3 viewDir = normalize(u_CameraPos.xyz - v_WorldPos);
	vec3 albedo  = u_Color.rgb * texture(u_Texture, v_TexCoord).rgb;

	// Ambient
	vec3 result = u_AmbientColor.xyz * albedo * ao;

	// Directional light
	if (u_LightInfo.x != 0)
		result += CalcDirLight(norm, viewDir, shininess) * albedo;

	// Point lights
	int numPoint = min(u_LightInfo.y, MAX_POINT_LIGHTS);
	for (int i = 0; i < numPoint; i++)
		result += CalcPointLight(i, norm, viewDir, shininess) * albedo;

	// Reinhard tone mapping to prevent blown-out highlights.
	result = result / (result + vec3(1.0));

	float alpha = u_Color.a * texture(u_Texture, v_TexCoord).a;
	o_Color    = vec4(result, alpha);
	o_EntityID = u_EntityID;
}
