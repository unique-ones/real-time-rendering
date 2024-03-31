#version 450
layout (location = 0) in vec3 attrib_position;
layout (location = 1) in vec3 attrib_color;
layout (location = 2) in vec3 attrib_normal;
layout (location = 3) in vec2 attrib_uv;

layout (location = 0) out vec3 passed_color;

layout (push_constant) uniform Push {
    // projection * view * model
    mat4 transform; 
    mat4 normal;
} push;

const vec3 DIRECTION_TO_LIGHT = normalize(vec3(1.0, -3.0, -1.0));
const float AMBIENT_COLOR = 0.02f;

void main() {
    gl_Position = push.transform * vec4(attrib_position, 1.0);

    // Lighting
    vec3 normal_world_space = normalize(mat3(push.normal) * attrib_normal);
    float light_intensity = AMBIENT_COLOR + max(dot(normal_world_space, DIRECTION_TO_LIGHT), 0);
    passed_color = light_intensity * attrib_color;
}
