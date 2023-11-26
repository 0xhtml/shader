#version 460 core

const vec2 verts[4] = {vec2(-1, -1), vec2(1, -1), vec2(-1, 1), vec2(1, 1)};

void main() {
    gl_Position = vec4(verts[gl_VertexID], 0, 1);
}
