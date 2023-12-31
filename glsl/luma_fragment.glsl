#version 460 core

out vec4 color;

layout(binding = 0) uniform sampler2D image;

void main() {
    vec3 rgb = texelFetch(image, ivec2(gl_FragCoord.xy), 0).rgb;
    float luma = dot(rgb, vec3(0.2126, 0.7152, 0.0722));
    color = vec4(rgb, luma);
}
