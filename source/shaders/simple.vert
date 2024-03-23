#version 450
layout (location = 0) in vec3 attrib_position;
layout (location = 1) in vec3 attrib_color;
layout (location = 0) out vec3 passed_color;

layout (push_constant) uniform Push {
    mat4 transform;
    vec3 color;
} push;

void main() {
    gl_Position = push.transform * vec4(attrib_position, 1.0);
    passed_color = attrib_color;
}
