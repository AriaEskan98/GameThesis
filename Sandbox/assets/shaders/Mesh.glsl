// 3D Mesh Shader
// Renders a mesh with a flat colour. Normal data is passed through for future
// lighting support but is not used in shading yet.

#type vertex
#version 450 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

layout(location = 0) out vec3 v_Normal;
layout(location = 1) out vec3 v_WorldPos;
layout(location = 2) out vec2 v_TexCoord;

void main()
{
	mat3 normalMatrix = mat3(transpose(inverse(u_Transform)));
	v_Normal   = normalize(normalMatrix * a_Normal);
	v_WorldPos = vec3(u_Transform * vec4(a_Position, 1.0));
	v_TexCoord = a_TexCoord;

	gl_Position = u_ViewProjection * u_Transform * vec4(a_Position, 1.0);
}

#type fragment
#version 450 core

layout(location = 0) out vec4 o_Color;
layout(location = 1) out int  o_EntityID;

layout(location = 0) in vec3 v_Normal;
layout(location = 1) in vec3 v_WorldPos;
layout(location = 2) in vec2 v_TexCoord;

uniform vec4 u_Color;
uniform int  u_EntityID;

void main()
{
	o_Color    = u_Color;
	o_EntityID = u_EntityID;
}
