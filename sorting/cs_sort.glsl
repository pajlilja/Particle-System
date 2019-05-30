#version 450 core

uniform float pass;
uniform float two_stage;
uniform float pass_mod_stage;
uniform float two_stage_pms_1;

layout (std430, binding = 1) buffer data_buffer {
  float data[];
};

layout (local_size_x = 1024, local_size_y = 1, local_size_z = 1) in;

void main() {
  uint idx = gl_GlobalInvocationID.x;
  float j = floor(mod(float(idx), two_stage));
  if (j < pass_mod_stage || j > two_stage_pms_1) return;
  if (mod((j + pass_mod_stage) / pass, 2.) >= 1.) return;
  uint partner = idx + uint(pass);  
  float self_v = data[idx];
  float partner_v = data[partner];
  if (partner_v < self_v) {
    data[partner] = self_v;
    data[idx] = partner_v;
  }
}
