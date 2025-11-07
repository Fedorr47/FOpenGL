#version 330 core
layout (location=0) in vec3 pos;
layout (location=1) in vec2 tex;
layout (location=2) in vec3 norm;

out vec2 TexCoord;
out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(pos,1.0);
    TexCoord = tex;
    mat3 N = transpose(inverse(mat3(model)));
    Normal = normalize(N * norm);
    FragPos = vec3(model * vec4(pos,1.0));
}
