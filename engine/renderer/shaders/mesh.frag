#version 450

layout(location = 0) in vec4 fragColor;
layout(location = 1) in vec3 fragNormal;
layout(location = 2) in vec2 fragTexCoord;

layout(push_constant) uniform PushConstants {
    mat4 viewProjection;
    mat4 model;
    vec4 color;
} pc;

layout(location = 0) out vec4 outColor;

void main() {
    // Simple hemisphere lighting
    vec3 N = normalize(fragNormal);
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diffuse = max(dot(N, lightDir), 0.0);
    float ambient = 0.25;
    float light = ambient + diffuse * 0.75;

    outColor = vec4(fragColor.rgb * light, fragColor.a);
}
