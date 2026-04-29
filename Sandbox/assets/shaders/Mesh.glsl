// 3D Mesh Shader — matches phong.frag from reference project

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
	T = normalize(T - dot(T, N) * N); // Gram-Schmidt re-orthogonalise
	vec3 B = normalize(cross(N, T));

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
	vec4 Color;
};

struct PointLightGPU
{
	vec4 Position;    // xyz = position, w = intensity
	vec4 Color;       // xyz = colour,   w = constant attenuation
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
vec3 CalcDirLight(vec3 N, vec3 V, vec3 albedo, float shininess, vec3 F0)
{
	vec3  L         = normalize(-u_DirLight.Direction.xyz);
	float intensity = u_DirLight.Direction.w;
	vec3  H         = normalize(L + V);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float spec  = pow(NdotH, shininess);

	vec3 diffuse  = NdotL * u_DirLight.Color.rgb * intensity * albedo;
	vec3 specular = spec  * u_DirLight.Color.rgb * intensity * F0;
	return diffuse + specular;
}

// ---------------------------------------------------------------------------
vec3 CalcPointLight(int i, vec3 N, vec3 V, vec3 albedo, float shininess, vec3 F0)
{
	vec3  toLight   = u_PointLights[i].Position.xyz - v_WorldPos;
	vec3  L         = normalize(toLight);
	float intensity = u_PointLights[i].Position.w;
	float dist      = length(toLight);
	vec3  H         = normalize(L + V);

	float constant  = u_PointLights[i].Color.w;
	float linear    = u_PointLights[i].Attenuation.x;
	float quadratic = u_PointLights[i].Attenuation.y;
	float atten     = 1.0 / (constant + linear * dist + quadratic * dist * dist);

	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float spec  = pow(NdotH, shininess);

	vec3 diffuse  = NdotL * u_PointLights[i].Color.rgb * intensity * albedo;
	vec3 specular = spec  * u_PointLights[i].Color.rgb * intensity * F0;
	return atten * (diffuse + specular);
}

// ---------------------------------------------------------------------------
void main()
{
	// --- Normal ---
	vec3 N;
	if (u_HasNormalMap != 0)
	{
		vec3 n = texture(u_NormalMap, v_TexCoord).rgb * 2.0 - vec3(1.0);
		N = normalize(v_TBN * n);
	}
	else
	{
		N = normalize(v_Normal);
	}

	// R = roughness, G = metalness. AO left at 1.0 — a roughness-only map
	// (OBJ map_Ns, type 7) has B=0 which would zero out ambient entirely.
	float roughness = 0.5;
	float metalness = 0.0;
	float ao        = 1.0;
	if (u_HasRMAMap != 0)
	{
		vec3 rma = texture(u_RMAMap, v_TexCoord).rgb;
		roughness = rma.r;
		metalness = rma.g;
	}

	float shininess = mix(8.0, 128.0, 1.0 - roughness);

	// Diffuse loaded as GL_SRGB8 — OpenGL auto-linearises on sample.
	vec3 albedo = u_Color.rgb * texture(u_Texture, v_TexCoord).rgb;

	// Metalness-based specular reflectance (matches reference project)
	vec3 F0 = mix(vec3(0.04), albedo, metalness);

	vec3 V = normalize(u_CameraPos.xyz - v_WorldPos);

	// Ambient — AO modulates ambient only, same as reference
	vec3 result = u_AmbientColor.xyz * albedo * ao;

	// Directional light
	if (u_LightInfo.x != 0)
		result += CalcDirLight(N, V, albedo, shininess, F0);

	// Point lights
	int numPoint = min(u_LightInfo.y, MAX_POINT_LIGHTS);
	for (int i = 0; i < numPoint; i++)
		result += CalcPointLight(i, N, V, albedo, shininess, F0);

	// Tone mapping (Reinhard) — same as reference project
	result = result / (result + vec3(1.0));

	// No manual gamma correction — GL_FRAMEBUFFER_SRGB handles linear→sRGB output.
	float alpha = u_Color.a * texture(u_Texture, v_TexCoord).a;
	o_Color    = vec4(result, alpha);
	o_EntityID = u_EntityID;
}
