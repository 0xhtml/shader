#version 460 core

out vec4 color;

layout(location = 0) uniform sampler2D image;

void main() {
    color = vec4(texture(image, gl_FragCoord.xy / 500).rgb, 1);
}
