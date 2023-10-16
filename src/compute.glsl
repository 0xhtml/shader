#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(binding = 0, rgba8) uniform image2D image;

layout(std430, binding = 1) buffer T {
    int longest;
} SSBO;

void main() {
    /* int longest = 0; */
    int count = 0;
    bool last = false;

    for (ivec2 pos = ivec2(gl_GlobalInvocationID.x, 0); pos.y < 500; ++pos.y) {
        if (imageLoad(image, pos).a > .25) {
            ++count;
            last = true;
        } else {
            if (last) {
                /* if (count > longest) longest = count; */
                atomicMax(SSBO.longest, count);
                count = 0;
            }
            last = false;
        }
    }

    /* if (last && count > longest) longest = count; */ 

    if (last) atomicMax(SSBO.longest, count);
}
