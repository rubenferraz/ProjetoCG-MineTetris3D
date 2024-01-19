#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 camPos;
uniform int blockType;

uniform sampler2D blockTex;
uniform sampler2D normalTex;

void main() {
    //vec3 normal = normalize(vertex_normal);
    vec3 normal = texture(normalTex, vertex_tex).rgb;
    normal = normalize(normal * 2.0 - 1.0);
    normal = normalize(normal * 0.2 + vertex_normal);
    vec3 lightPos = vec3(5, 5, 5);
    vec3 lightDir = normalize(lightPos - vertex_pos);
    float diffuseFactor = max(dot(lightDir, normal), 0.0);
    
    vec4 diffuse = diffuseFactor * vec4(1, 1, 1, 1);
    
    vec3 viewDir = normalize(camPos - vertex_pos);
    vec3 reflectionDir = normalize(lightDir + camPos);
    float specularLight = pow(max(dot(viewDir, reflectionDir), 0.0), 8);
    vec4 spec = specularLight * vec4(1, 1, 1, 1);
    
    color = texture(blockTex, vertex_tex);
    
    if (blockType == 1) {
        color += vec4(vec3(0.1, 0.0, 0.6), 1);
    } else if (blockType == 2) {
        color += vec4(vec3(0.5, 0.2, 0.8), 1); //0.0 inicial
    } else if (blockType == 3) {
        color += vec4(vec3(0.1, 0.2, 0.3), 1);
    } else if (blockType == 4) {
        color += vec4(vec3(0.4, -0.1, -0.1), 1);
    } else if (blockType == 5) {
        color += vec4(vec3(0.3, -0.3, 0.3), 1);
    } else if (blockType == 6) {
        color += vec4(vec3(0.1, -0.05, 0.1), 1);
    } else if (blockType == 7) {
        color += vec4(vec3(0.8, 0.0, 0.0), 1);
    } else if (blockType == -1) { //ghost block
        color += vec4(-0.2, -0.4, -0.7, -0.8); //white transparent
    } else if (blockType == -2) { //border
        color = vec4(vertex_pos, 1.0) * 0.2;
        color += texture(blockTex, vertex_tex) * 0.3;
        color += 0.3;
        color.a = 1;
        if (color.g > 0.6) color.g -= 0.1;
        if (color.r < 0.2) color.r += 0.1;
        if (color.b < 0.2) color.b += 0.2;
    } else if (blockType == -3) { //etc blocks
        color += vec4(vec3(0.1, 0.2, 0.2), 1);
    }
    color *= (diffuse + spec);
    color -= vec4(0.1, 0.1, 0.1, 0.1);
    if (blockType != -1) color.a = 1;
}
