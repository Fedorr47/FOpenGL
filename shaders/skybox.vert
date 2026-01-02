#version 330

layout (location = 0) in vec3 pos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
	TexCoords = pos;
	vec4 lpos = projection * view * vec4(aPos, 1.0);
	gl_Position = lpos.xyww;
}