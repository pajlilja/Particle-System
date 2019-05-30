#version 450

uniform float time;
uniform float delta_time;
uniform vec3 prev_position;
uniform vec3 curr_position;
uniform float ttl;

struct particle {
  vec3 position;
  vec3 velocity;
  float start_ttl;
  vec3 start_velocity;
  float ttl;
};

layout (std140, binding = 1) buffer particle_storage {
  particle particles[];
};

layout (std430, binding = 2) buffer dead_list {
  uint dead[];
};

layout (std430, binding = 3) buffer alive_list {
  uint alive[];
};

layout (binding = 0, offset = 0) uniform atomic_uint alive_count;
layout (binding = 0, offset = 4) uniform atomic_uint alive_count_prev;
layout (binding = 0, offset = 8) uniform atomic_uint dead_count;

float rand(vec2 p) {
  return fract(sin(dot(p.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

layout (local_size_x = 64) in;
void main() {
  uint g_idx = atomicCounterDecrement(dead_count);
  /* if underflow we know this thread shouldn't continue */
  if (g_idx > uint(10000000)) {
    return;
  }
  /* spawn new particle */
  uint p_idx = dead[g_idx];
  float a = abs(particles[p_idx].ttl / delta_time);
  particles[p_idx].position = prev_position + a * (prev_position - curr_position);
  particles[p_idx].velocity = particles[p_idx].start_velocity * 0.1;
  particles[p_idx].position += particles[p_idx].velocity * abs(particles[p_idx].ttl);
  particles[p_idx].ttl = rand(vec2(time, float(p_idx))) * ttl;
  particles[p_idx].start_ttl = particles[p_idx].ttl;
  alive[atomicCounterIncrement(alive_count)] = p_idx;
}
