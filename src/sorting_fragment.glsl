#version 460 core

out vec4 color;

layout(location = 0) uniform sampler2D image;
layout(location = 1) uniform sampler2D depth;
layout(location = 2) uniform int pass;
layout(location = 3) uniform float threshold;

void main() {
    vec4 self_color = texture(image, gl_FragCoord.xy / 500);
    float self_depth = texture(depth, gl_FragCoord.xy / 500).x;
    int invert = int(gl_FragCoord.y + pass) % 2;

    if (self_depth > threshold || int(gl_FragCoord.y) == 499 * (1 - invert)) {
        color = self_color;
    } else {
        vec4 next_color = texture(image, (gl_FragCoord.xy + vec2(0, 1 - invert * 2)) / 500);
        float next_depth = texture(depth, (gl_FragCoord.xy + vec2(0, 1 - invert * 2)) / 500).x;
        if (next_depth > threshold) {
            color = self_color;
        } else {
            color = (invert == 1 ? (self_color.a < next_color.a) : (self_color.a > next_color.a)) ? self_color : next_color;
        }
    }
}
