#version 330 core

layout (location = 0) in vec3 v_position;

void main() {
  gl_Position = vec4(v_position, 1.);
}