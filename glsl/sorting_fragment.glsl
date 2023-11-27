#version 460 core

out vec4 color;

layout(location = 0) uniform sampler2D image;
layout(location = 1) uniform float threshold;
layout(location = 2) uniform int k;
layout(location = 3) uniform int j;

void main() {
    int i = int(gl_FragCoord.y);
    int l = i ^ j;

    vec4 self_color = texelFetch(image, ivec2(gl_FragCoord.x, i), 0);
    vec4 next_color = texelFetch(image, ivec2(gl_FragCoord.x, l), 0);

    if ((i & k) == 0 ^^ l > i) {
        color = self_color.a < next_color.a ? self_color : next_color;
    } else {
        color = self_color.a > next_color.a ? self_color : next_color;
    }
}
