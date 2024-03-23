#version 450
layout (location = 0) in vec2 attrib_position;
layout (location = 1) in vec3 attrib_color;

layout (location = 0) out vec3 passed_color;

void main() {
    gl_Position = vec4(attrib_position, 0.0, 1.0);
    passed_color = attrib_color;
}
