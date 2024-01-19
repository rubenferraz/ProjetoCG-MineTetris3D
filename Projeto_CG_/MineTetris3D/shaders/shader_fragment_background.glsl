#version 330 core
out vec3 color;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 camPos;

uniform sampler2D backgroundTex;

void main() {
    color = texture(backgroundTex, vertex_tex).rgb;
}
