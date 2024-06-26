#version 450
layout (location = 0) out vec4 out_color;
layout (location = 0) in vec3 passed_color;

layout (push_constant) uniform Push {
    // projection * view * mesh
    mat4 transform; 
    mat4 normal;
} push;

void main() {
    out_color = vec4(passed_color, 1.0);
}
