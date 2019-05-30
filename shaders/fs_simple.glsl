#version 430 core

layout(location = 0) out vec4 out_albedo;

uniform sampler2D color_texture;
uniform sampler2D bloom_texture;

uniform vec2 res;
uniform float exposure;
uniform float gamma;
uniform bool has_bloom;

void main() {
	int loopValue = 2;
	vec2 uv = gl_FragCoord.xy/res;
	vec3 color = texture(color_texture, uv).xyz;
	vec3 bloom = texture(bloom_texture, uv).xyz;
	if(has_bloom) {
		color += bloom;
	}
	color = vec3(1.0) - exp(-color * exposure);
	out_albedo = vec4(pow(color, vec3(1./gamma)), 1.);
}
