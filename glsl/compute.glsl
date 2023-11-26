#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(location = 0) uniform float threshold;
layout(binding = 1) uniform sampler2D depth;

layout(std430, binding = 1) buffer T {
    int longest;
} SSBO;

void main() {
    int count = 0;
    bool last = false;

    for (ivec2 pos = ivec2(gl_GlobalInvocationID.x, 0); pos.y < 500; ++pos.y) {
        if (texture(depth, pos / 500.0).r <= threshold) {
            ++count;
            last = true;
        } else if (last) {
            atomicMax(SSBO.longest, count);
            count = 0;
            last = false;
        }
    }

    if (last) atomicMax(SSBO.longest, count);
}
