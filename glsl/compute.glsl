#version 460 core

layout(local_size_x = 1, local_size_y = 1) in;

layout(location = 0) uniform float threshold;

layout(binding = 0, rgba16) uniform image2D FBO;
layout(binding = 1) uniform sampler2D depth;

void main() {
    int size = imageSize(FBO).y;
    float lmax = 0;
    float nmax = 0;
    for (ivec2 pos = ivec2(gl_GlobalInvocationID.x, size - 1); pos.y >= 0; --pos.y) {
        vec4 color = imageLoad(FBO, pos);
        color.a /= size;
        if (texelFetch(depth, pos, 0).r <= threshold) {
            color.a += lmax;
            if (color.a > nmax) nmax = color.a;
        } else {
            if (nmax > lmax) lmax = nmax;
            lmax += 1.0 / size;
            color.a = lmax;
        }
        imageStore(FBO, pos, color);
    }
}
