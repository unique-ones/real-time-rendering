#version 450
layout (location = 0) in vec2 attrib_position;

void main() {
    gl_Position = vec4(attrib_position, 0.0, 1.0);
}
