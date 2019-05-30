#version 450

in vec3 f_velocity;
in float f_ttl;
in float f_start_ttl;

out vec4 out_color;

uniform vec3 start_color;
uniform vec3 end_color;

void main(){
  out_color = vec4(mix(end_color, start_color, f_ttl / f_start_ttl), 1.);
}
