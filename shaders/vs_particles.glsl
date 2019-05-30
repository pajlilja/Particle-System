#version 450

layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_velocity;
layout(location = 2) in float v_ttl;
layout(location = 3) in float v_start_ttl;

uniform mat4 projection_matrix;
uniform mat4 view_matrix;

out vec3 f_velocity;
out float f_ttl;
out float f_start_ttl;

void main(){
	f_velocity = v_velocity;
	f_ttl = v_ttl;
	f_start_ttl = v_start_ttl;
	vec4 p = projection_matrix*view_matrix*vec4(v_position, 1.0);
	gl_PointSize = (1. - abs(p.z / p.w)) * 20.;
	gl_Position = p;
}
